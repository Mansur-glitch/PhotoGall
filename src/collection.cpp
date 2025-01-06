// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2025 Mansur Mukhametzyanov
#include "collection.hpp"

Group::Group(uint32_t id, uint32_t sampleIndex, uint32_t begin, uint32_t size)
    : m_id(id), m_sample(sampleIndex), m_begin(begin), m_size(size) {}

uint32_t Group::begin() const noexcept {
  return m_begin;
}

uint32_t Group::end() const noexcept {
  return m_begin + m_size;
}

uint32_t Group::size() const noexcept {
  return m_size;
}

uint32_t Group::getSample() const noexcept {
  return m_sample;
}

uint32_t Group::id() const noexcept {
  return m_id;
}

void Group::setBegin(uint32_t position) noexcept {
  m_begin = position;
}

void Group::setSize(uint32_t size) noexcept {
  m_size = size;
}

GroupLayoutBase::GroupLayoutBase(QList<uint32_t>& indexes)
    : m_data(indexes), m_groups(), m_orderedIds() {}

const Group& GroupLayoutBase::getById(uint32_t id) const {
  return m_groups[id];
}

const Group& GroupLayoutBase::getByOrder(uint32_t num) const {
  return m_groups[m_orderedIds[num]];
}

uint32_t GroupLayoutBase::getIdAt(uint32_t num) const {
  return m_orderedIds[num];
}

uint32_t GroupLayoutBase::countGroups() const {
  return m_groups.size();
}

uint32_t GroupLayoutBase::size() const {
  return m_data.size();
}

void GroupLayoutBase::clear() {
  m_groups.clear();
  m_orderedIds.clear();
}

using Sublayout = ArrangedGroupLayout::Sublayout;

Sublayout::Sublayout(ArrangedGroupLayout* layout)
    : m_layout(layout), m_orderedIds(), m_begin(0), m_size(0), m_nGroups(0) {}

Group& Sublayout::getByOrder(uint32_t num) {
  return m_layout->m_groups[m_orderedIds[num]];
}

uint32_t Sublayout::getIdAt(uint32_t num) const {
  return m_orderedIds[num];
}

uint32_t Sublayout::countGroups() const {
  return m_nGroups;
}

std::optional<uint32_t> Sublayout::getIdBySample(
    uint32_t sample,
    const Predicate2<uint32_t>& sameGroupPred) {
  for (uint32_t groupNum = 0; groupNum < m_nGroups; ++groupNum) {
    Group& g = getByOrder(groupNum);
    if (sameGroupPred(sample, g.getSample())) {
      return g.id();
      break;
    }
  }
  return {};
}

QList<uint32_t> Sublayout::arrange_getDataMarkup() {
  QList<uint32_t> groupMarks(m_size);
  // Mark previously ordered data
  for (uint32_t groupNum = 0; groupNum < m_nGroups; ++groupNum) {
    const Group& g = getByOrder(groupNum);
    const uint32_t begin = g.begin() - m_begin;
    const uint32_t end = g.end() - m_begin;
    for (uint32_t i = begin; i < end; ++i) {
      groupMarks[i] = g.id();
    }
  }
  return groupMarks;
}

Group& Sublayout::arrange_append(uint32_t groupSample) {
  uint32_t newGroupId = m_layout->m_groups.size();
  m_layout->m_groups.append(Group{newGroupId, groupSample, 0, 1});
  m_orderedIds.append(newGroupId);
  m_layout->m_orderedIds.append(newGroupId);
  m_layout->m_idOrderPosition.append(newGroupId);
  m_nGroups += 1;
  return m_layout->m_groups[newGroupId];
}

void Sublayout::arrange_sortGroups(const Predicate2<uint32_t>& groupLessPred) {
  std::ranges::sort(m_orderedIds, groupLessPred, [this](uint32_t i) {
    return m_layout->m_groups[i].getSample();
  });
}

void Sublayout::arrange_storeLocalGroupOrder() {
  for (uint32_t i = 0; i < m_orderedIds.size(); ++i) {
    m_layout->m_idOrderPosition[m_orderedIds[i]] = i;
  }
}

void Sublayout::arrange_writeData(QList<uint32_t> dataMarkup) {
  QList<uint32_t> inGroupPosition(m_nGroups);
  inGroupPosition.fill(0);
  QList<uint32_t> buf(m_size);
  for (uint32_t i = 0; i < m_size; ++i) {
    const uint32_t groupId = dataMarkup[i];
    // Main layout's field temporarly used to store local group order
    const uint32_t groupOrderNum = m_layout->m_idOrderPosition[groupId];
    const uint32_t elementPos =
        m_layout->m_groups[groupId].begin() + inGroupPosition[groupOrderNum];
    buf[elementPos - m_begin] = m_layout->m_data[i + m_begin];
    inGroupPosition[groupOrderNum] += 1;
  }
  std::copy(buf.begin(), buf.end(), m_layout->m_data.begin() + m_begin);
}

void Sublayout::arrange_distributeNewData(
    QList<uint32_t>& currentMarkup,
    uint32_t expandedSize,
    const Predicate2<uint32_t>& sameGroupPred) {
  const uint32_t newDataBegin = currentMarkup.size();
  const uint32_t newDataEnd = newDataBegin + expandedSize;
  currentMarkup.resize(newDataEnd);
  for (uint32_t i = newDataBegin; i < newDataEnd; ++i) {
    const uint32_t globalIndex = i + m_begin;
    const uint32_t element = m_layout->m_data[globalIndex];
    std::optional<uint32_t> optGroupId = getIdBySample(element, sameGroupPred);
    if (optGroupId) {
      Group& g = m_layout->m_groups[optGroupId.value()];
      g.setSize(g.size() + 1);
      currentMarkup[i] = optGroupId.value();
    } else {
      currentMarkup[i] = arrange_append(element).id();
    }
  }
}

void Sublayout::arrange_setSublayoutPosition(uint32_t begin, uint32_t size) {
  m_begin = begin;
  m_size = size;
}

void Sublayout::arrange_setGroupBeginnings() {
  uint32_t i = 0;
  for (uint32_t groupNum = 0; groupNum < m_nGroups; ++groupNum) {
    Group& g = getByOrder(groupNum);
    g.setBegin(i + m_begin);
    i += g.size();
  }
}

void Sublayout::arrange(uint32_t begin,
                        uint32_t size,
                        const Predicate2<uint32_t>& sameGroupPred,
                        const Predicate2<uint32_t>& groupLessPred) {
  const uint32_t expansionSize = size - m_size;
  if (expansionSize > 0) {
    QList<uint32_t> dataMarkup = arrange_getDataMarkup();
    arrange_setSublayoutPosition(begin, size);
    const uint32_t nGroupsBefore = m_nGroups;
    arrange_distributeNewData(dataMarkup, expansionSize, sameGroupPred);
    if (m_nGroups > nGroupsBefore) {
      arrange_sortGroups(groupLessPred);
    }
    arrange_storeLocalGroupOrder();
    arrange_setGroupBeginnings();
    arrange_writeData(dataMarkup);
  } else {
    arrange_setSublayoutPosition(begin, size);
    arrange_setGroupBeginnings();
  }
}

ArrangedGroupLayout::ArrangedGroupLayout(QList<uint32_t>& indexes)
    : GroupLayoutBase(indexes), m_idOrderPosition(), m_sublayouts() {}

uint32_t ArrangedGroupLayout::getOrderById(uint32_t id) const {
  return m_idOrderPosition[id];
}

void ArrangedGroupLayout::clear() {
  GroupLayoutBase::clear();
  m_idOrderPosition.clear();
  m_sublayouts.clear();
}

void ArrangedGroupLayout::arrange(const GroupLayoutBase& inputLayout,
                                  const Predicate2<uint32_t>& sameGroupPred,
                                  const Predicate2<uint32_t>& groupLessPred) {
  const uint32_t nSublayoutsBefore = m_sublayouts.size();
  const uint32_t nSublayoutsAfter = inputLayout.countGroups();
  for (uint32_t i = 0; i < nSublayoutsBefore; ++i) {
    const Group& g = inputLayout.getById(i);
    m_sublayouts[i].arrange(g.begin(), g.size(), sameGroupPred, groupLessPred);
  }
  for (uint32_t i = nSublayoutsBefore; i < nSublayoutsAfter; ++i) {
    const Group& g = inputLayout.getById(i);
    m_sublayouts.emplaceBack<ArrangedGroupLayout*>(this);
    m_sublayouts.last().arrange(g.begin(), g.size(), sameGroupPred,
                                groupLessPred);
  }

  m_orderedIds.resize(m_groups.size());
  m_idOrderPosition.resize(m_groups.size());
  uint32_t orderOffset = 0;
  for (uint32_t n = 0; n < nSublayoutsAfter; ++n) {
    uint32_t i = inputLayout.getIdAt(n);
    for (uint32_t m = 0; m < m_sublayouts[i].countGroups(); ++m) {
      const uint32_t id = m_sublayouts[i].getIdAt(m);
      const uint32_t position = orderOffset + m;
      m_orderedIds[position] = id;
      m_idOrderPosition[id] = position;
    }
    orderOffset += m_sublayouts[i].countGroups();
  }
}

void ArrangedGroupLayout::arrange(const Predicate2<uint32_t>& sameGroupPred,
                                  const Predicate2<uint32_t>& groupLessPred) {
  if (m_sublayouts.empty()) {
    m_sublayouts.emplaceBack<ArrangedGroupLayout*>(this);
  }
  m_sublayouts[0].arrange(0, m_data.size(), sameGroupPred, groupLessPred);
  m_orderedIds.resize(m_groups.size());
  m_idOrderPosition.resize(m_groups.size());
  for (uint32_t n = 0; n < m_groups.size(); ++n) {
    const uint32_t id = m_sublayouts[0].getIdAt(n);
    const uint32_t position = n;
    m_orderedIds[position] = id;
    m_idOrderPosition[id] = position;
  }
}

FilteredGroupLayout::FilteredGroupLayout(QList<uint32_t>& indexes)
    : GroupLayoutBase(indexes), m_filtrationInfo() {}

uint32_t FilteredGroupLayout::filterRange(uint32_t begin,
                                          uint32_t end,
                                          const GroupFiltrationInfo& finfo,
                                          const Predicate1<uint32_t>& filter) {
  const QList<uint32_t>::iterator expBegIt =
      m_data.begin() + begin + finfo.m_unfilteredSize;
  const QList<uint32_t>::iterator endIt = m_data.begin() + end;
  QList<uint32_t>::iterator partIt =
      std::stable_partition(expBegIt, endIt, filter);
  const uint32_t nItemsAdded = partIt - expBegIt;

  if (nItemsAdded > 0) {
    const QList<uint32_t>::iterator inappBegIt =
        m_data.begin() + begin + finfo.m_filteredSize;
    std::rotate(inappBegIt, expBegIt, partIt);
  }
  return nItemsAdded;
}

Group& FilteredGroupLayout::appendGroup(uint32_t begin, uint32_t size) {
  const uint32_t id = m_groups.size();
  const uint32_t sample = m_data[begin];
  m_groups.append(Group(id, sample, begin, size));
  m_orderedIds.append(id);
  return m_groups.last();
}

void FilteredGroupLayout::filter(const Predicate1<uint32_t>& filter) {
  if (m_filtrationInfo.empty()) {
    m_filtrationInfo.append({0, 0, 0});
  }
  GroupFiltrationInfo& finfo = m_filtrationInfo[0];
  const uint32_t nAdded = filterRange(0, m_data.size(), finfo, filter);
  finfo.m_unfilteredSize = m_data.size();
  finfo.m_filteredSize += nAdded;

  if (nAdded > 0) {
    if (!m_groups.empty()) {
      m_groups[finfo.m_filteredGroupId].setSize(finfo.m_filteredSize);
    } else {
      finfo.m_filteredGroupId = appendGroup(0, finfo.m_filteredSize).id();
    }
  }
}

void FilteredGroupLayout::filter(const GroupLayoutBase& inputLayout,
                                 const Predicate1<uint32_t>& filter) {
  const uint32_t nOldGroups = m_filtrationInfo.size();
  m_orderedIds.clear();
  m_filtrationInfo.resize(inputLayout.countGroups());
  for (uint32_t groupNum = 0; groupNum < inputLayout.countGroups();
       ++groupNum) {
    const Group& g = inputLayout.getByOrder(groupNum);
    GroupFiltrationInfo& finfo = m_filtrationInfo[g.id()];
    const bool hasFilteredGroup = finfo.m_filteredSize > 0;
    if (hasFilteredGroup) {
      m_groups[finfo.m_filteredGroupId].setBegin(g.begin());
      m_orderedIds.append(finfo.m_filteredGroupId);
    }
    if (g.size() == finfo.m_unfilteredSize) {
      continue;
    }
    const uint32_t nAdded = filterRange(g.begin(), g.end(), finfo, filter);
    finfo.m_unfilteredSize = g.size();
    finfo.m_filteredSize += nAdded;

    if (nAdded > 0) {
      if (hasFilteredGroup) {
        m_groups[finfo.m_filteredGroupId].setSize(finfo.m_filteredSize);
      } else {
        finfo.m_filteredGroupId =
            appendGroup(g.begin(), finfo.m_filteredSize).id();
      }
    }
  }
}

void FilteredGroupLayout::clear() {
  GroupLayoutBase::clear();
  m_filtrationInfo.clear();
}

CollectionView::CollectionView(QList<uint32_t>& indexes,
                               const GroupLayoutBase* inputLayout)
    : m_indexes(indexes), m_inputLayout(inputLayout) {}

CollectionView::~CollectionView() {}

void CollectionView::setInput(const GroupLayoutBase* input) {
  m_inputLayout = input;
}

const GroupLayoutBase* CollectionView::outputLayout() const {
  return m_inputLayout;
}

NullView::NullView(QList<uint32_t>& indexes, const GroupLayoutBase* inputLayout)
    : CollectionView(indexes, inputLayout) {}

void NullView::update() {}

void NullView::reset() {}

GroupingView::GroupingView(Predicate2<uint32_t> sameGroupPred,
                           Predicate2<uint32_t> lessGroupPred,
                           QList<uint32_t>& indexes,
                           const GroupLayoutBase* inputLayout)
    : CollectionView(indexes, inputLayout),
      m_outputLayout(indexes),
      m_sameGroupPred(std::move(sameGroupPred)),
      m_lessGroupPred(std::move(lessGroupPred)) {}

void GroupingView::update() {
  if (m_inputLayout) {
    m_outputLayout.arrange(*m_inputLayout, m_sameGroupPred, m_lessGroupPred);
  } else {
    m_outputLayout.arrange(m_sameGroupPred, m_lessGroupPred);
  }
}

void GroupingView::reset() {
  m_outputLayout.clear();
}

const ArrangedGroupLayout* GroupingView::outputLayout() const {
  return &m_outputLayout;
}

SortingView::SortingView(Predicate2<uint32_t> elementLessPred,
                         QList<uint32_t>& indexes,
                         const GroupLayoutBase* inputLayout)
    : CollectionView(indexes, inputLayout),
      m_byIndexLessCompare(std::move(elementLessPred)) {}

void SortingView::sortPartiallySorted(uint32_t begin,
                                      uint32_t end,
                                      uint32_t unsortedBegin) {
  QList<uint32_t>::iterator begIt = m_indexes.begin() + begin;
  QList<uint32_t>::iterator midIt = m_indexes.begin() + unsortedBegin;
  QList<uint32_t>::iterator endIt = m_indexes.begin() + end;
  std::ranges::sort(midIt, endIt, m_byIndexLessCompare);
  std::inplace_merge(begIt, midIt, endIt, m_byIndexLessCompare);
}

void SortingView::update() {
  if (m_inputLayout == nullptr) {
    if (m_groupSizes.empty()) {
      m_groupSizes.append(0);
    }
    if (m_groupSizes[0] == 0) {
      std::ranges::sort(m_indexes, m_byIndexLessCompare);
    } else {
      sortPartiallySorted(0, m_indexes.size(), m_groupSizes[0]);
    }
    m_groupSizes[0] = m_indexes.size();
    return;
  }

  m_groupSizes.resize(m_inputLayout->countGroups(), 0);
  for (uint32_t groupId = 0; groupId < m_inputLayout->countGroups();
       ++groupId) {
    const Group& g = m_inputLayout->getById(groupId);
    if (g.size() == m_groupSizes[groupId]) {
      continue;
    }
    if (m_groupSizes[groupId] == 0) {
      std::ranges::sort(m_indexes.begin() + g.begin(),
                        m_indexes.begin() + g.end(), m_byIndexLessCompare);
    } else {
      sortPartiallySorted(g.begin(), g.end(),
                          g.begin() + m_groupSizes[groupId]);
    }
    m_groupSizes[groupId] = g.size();
  }
}

void SortingView::reset() {
  m_groupSizes.clear();
}

FilteringView::FilteringView(Predicate1<uint32_t> filterPred,
                             QList<uint32_t>& indexes,
                             const GroupLayoutBase* inputLayout)
    : CollectionView(indexes, inputLayout),
      m_outputLayout(indexes),
      m_filterByIndex(filterPred) {}

FilteringView::FilteringView(QList<uint32_t>& indexes,
                             const GroupLayoutBase* inputLayout)
    : CollectionView(indexes, inputLayout),
      m_filterByIndex(),
      m_outputLayout(indexes) {}

void FilteringView::update() {
  if (m_inputLayout) {
    m_outputLayout.filter(*m_inputLayout, m_filterByIndex);
  } else {
    m_outputLayout.filter(m_filterByIndex);
  }
}

void FilteringView::reset() {
  m_outputLayout.clear();
}

const FilteredGroupLayout* FilteringView::outputLayout() const {
  return &m_outputLayout;
}

RemovingView::RemovingView(QList<uint32_t>& indexes,
                           const QBitArray& deletedMask,
                           const GroupLayoutBase* inputLayout)
    : FilteringView(indexes, inputLayout) {
  m_filterByIndex = Predicate1<uint32_t>{
      [&deletedMask](const uint32_t& index) { return deletedMask[index]; }};
}
