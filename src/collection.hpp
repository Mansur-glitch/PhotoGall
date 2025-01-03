#pragma once
#include <QBitArray>
#include <QList>

#include <concepts>
#include <cstdint>
#include <optional>
#include <ranges>

template <class ViewTy>
  requires std::ranges::view<ViewTy> && requires(ViewTy v) {
    { *v.begin() } -> std::convertible_to<std::uint32_t>;
    { *v.end() } -> std::convertible_to<std::uint32_t>;
  }
QList<uint32_t> indexListFromView(ViewTy view) {
  return QList<uint32_t>(view.begin(), view.end());
}

template <class PredTy, class ElementTy>
concept Predicate1Concept = std::predicate<PredTy, const ElementTy&>;

template <class PredTy, class ElementTy>
concept Predicate2Concept =
    std::predicate<PredTy, const ElementTy&, const ElementTy>;

template <class InvokableTy, class ElementTy>
concept ToStringFuncConcept =
    std::invocable<InvokableTy, const ElementTy&> &&
    std::copyable<InvokableTy> && requires(ElementTy el) {
      { InvokableTy{}(el) } -> std::same_as<QString>;
    };

template <class ElementTy>
using Predicate1 = std::function<bool(const ElementTy&)>;
template <class ElementTy>
using Predicate2 = std::function<bool(const ElementTy&, const ElementTy&)>;

class Group {
 public:
  Group(uint32_t id,
        uint32_t sampleIndex,
        uint32_t begin = 0,
        uint32_t size = 1);
  uint32_t begin() const noexcept;
  uint32_t end() const noexcept;
  uint32_t size() const noexcept;
  uint32_t getSample() const noexcept;
  uint32_t id() const noexcept;
  void setBegin(uint32_t position) noexcept;
  void setSize(uint32_t size) noexcept;

 private:
  uint32_t m_id{0};
  uint32_t m_sample{0};
  uint32_t m_begin{0};
  uint32_t m_size{0};
};

class GroupLayoutBase {
 public:
  GroupLayoutBase(QList<uint32_t>& indexes);
  const Group& getById(uint32_t id) const;
  const Group& getByOrder(uint32_t num) const;
  uint32_t getIdAt(uint32_t num) const;
  uint32_t countGroups() const;
  uint32_t size() const;
  void clear();

 protected:
  QList<uint32_t>& m_data;
  QList<Group> m_groups;
  QList<uint32_t> m_orderedIds;
};

class FilteredGroupLayout : public GroupLayoutBase {
  struct GroupFiltrationInfo {
    uint32_t m_unfilteredSize{0};
    uint32_t m_filteredSize{0};
    uint32_t m_filteredGroupId{0};
  };

 public:
  FilteredGroupLayout(QList<uint32_t>& indexes);
  void filter(const Predicate1<uint32_t>& filter);
  void filter(const GroupLayoutBase& inputLayout,
              const Predicate1<uint32_t>& filter);
  void clear();

 private:
  uint32_t filterRange(uint32_t begin,
                       uint32_t end,
                       const GroupFiltrationInfo& finfo,
                       const Predicate1<uint32_t>& filter);
  Group& appendGroup(uint32_t begin, uint32_t size);
  QList<GroupFiltrationInfo> m_filtrationInfo;
};

class ArrangedGroupLayout : public GroupLayoutBase {
 public:
  struct Sublayout {
   public:
    Sublayout(ArrangedGroupLayout* layout);
    uint32_t countGroups() const;
    Group& getByOrder(uint32_t num);
    uint32_t getIdAt(uint32_t num) const;
    std::optional<uint32_t> getIdBySample(
        uint32_t sample,
        const Predicate2<uint32_t>& sameGroupPred);
    // Side effect: expands m_groups, writes to data
    void arrange(uint32_t begin,
                 uint32_t size,
                 const Predicate2<uint32_t>& sameGroupPred,
                 const Predicate2<uint32_t>& groupLessPred);

   private:
    QList<uint32_t> arrange_getDataMarkup();
    Group& arrange_append(uint32_t groupSample);
    void arrange_distributeNewData(QList<uint32_t>& currentMarkup,
                                   uint32_t newDataSize,
                                   const Predicate2<uint32_t>& sameGroupPred);
    void arrange_sortGroups(const Predicate2<uint32_t>&);
    // Temporary set local group order to main layout's field. Used in write
    // data.
    void arrange_storeLocalGroupOrder();
    void arrange_setSublayoutPosition(uint32_t begin, uint32_t size);
    void arrange_setGroupBeginnings();
    void arrange_writeData(QList<uint32_t> dataMarkup);

    ArrangedGroupLayout* m_layout;
    QList<uint32_t> m_orderedIds;
    uint32_t m_begin;
    uint32_t m_size;
    uint32_t m_nGroups;
  };

  ArrangedGroupLayout(QList<uint32_t>& indexes);
  void clear();
  void arrange(const GroupLayoutBase& inputLayout,
               const Predicate2<uint32_t>& sameGroupPred,
               const Predicate2<uint32_t>& groupLessPred);
  void arrange(const Predicate2<uint32_t>& sameGroupPred,
               const Predicate2<uint32_t>& groupLessPred);
  uint32_t getOrderById(uint32_t id) const;

 private:
  QList<uint32_t> m_idOrderPosition;
  QList<Sublayout> m_sublayouts;
};

class CollectionView {
 public:
  CollectionView(QList<uint32_t>& indexes,
                 const GroupLayoutBase* inputLayout = nullptr);
  virtual ~CollectionView();
  virtual void update() = 0;
  virtual void reset() = 0;
  virtual void setInput(const GroupLayoutBase* input);
  virtual const GroupLayoutBase* outputLayout() const;

 protected:
  QList<uint32_t>& m_indexes;
  const GroupLayoutBase* m_inputLayout{nullptr};
};

class NullView : public CollectionView {
 public:
  NullView(QList<uint32_t>& indexes,
           const GroupLayoutBase* inputLayout = nullptr);
  void update() override;
  void reset() override;
};

class GroupingView : public CollectionView {
 public:
  GroupingView(Predicate2<uint32_t> sameGroupPred,
               Predicate2<uint32_t> lessGroupPred,
               QList<uint32_t>& indexes,
               const GroupLayoutBase* inputLayout = nullptr);
  virtual void update() override;
  virtual void reset() override;
  const ArrangedGroupLayout* outputLayout() const override;

 private:
  ArrangedGroupLayout m_outputLayout;
  Predicate2<uint32_t> m_sameGroupPred;
  Predicate2<uint32_t> m_lessGroupPred;
};

class SortingView : public CollectionView {
 public:
  SortingView(Predicate2<uint32_t> elementLessPred,
              QList<uint32_t>& indexes,
              const GroupLayoutBase* inputLayout = nullptr);
  virtual void update() override;
  virtual void reset() override;

 private:
  void sortPartiallySorted(uint32_t begin,
                           uint32_t end,
                           uint32_t unsortedBegin);
  Predicate2<uint32_t> m_byIndexLessCompare;
  QList<uint32_t> m_groupSizes;
};

class FilteringView : public CollectionView {
 public:
  FilteringView(Predicate1<uint32_t> filterPred,
                QList<uint32_t>& indexes,
                const GroupLayoutBase* inputLayout = nullptr);
  virtual void update() override;
  virtual void reset() override;
  const FilteredGroupLayout* outputLayout() const override;

 protected:
  FilteringView(QList<uint32_t>& indexes,
                const GroupLayoutBase* inputLayout = nullptr);
  Predicate1<uint32_t> m_filterByIndex;
  FilteredGroupLayout m_outputLayout;
};

class RemovingView : public FilteringView {
 public:
  RemovingView(QList<uint32_t>& indexes,
               const QBitArray& deletedMask,
               const GroupLayoutBase* inputLayout = nullptr);
};

template <class ElementTy>
class IndexedSource {
 public:
  IndexedSource() : m_data(), m_indexes(), m_presenceMarks() {}

  void clear() {
    m_data.clear();
    m_indexes.clear();
    m_presenceMarks.clear();
  }

  void addItems(QList<ElementTy> newItems) {
    const uint32_t oldSize = m_data.size();
    const uint32_t newSize = oldSize + newItems.size();
    m_data.append(newItems);
    std::ranges::iota_view<uint32_t, uint32_t> newIndexes(oldSize, newSize);
    m_indexes.append(indexListFromView(newIndexes));
    m_presenceMarks.resize(newSize);
    m_presenceMarks.fill(true, oldSize, newSize);
  }

  void markRemoved(uint32_t pos) { m_presenceMarks[pos] = false; }

  void shrinkData() {
    const uint32_t oldSize = m_data.size();
    const uint32_t newSize = m_presenceMarks.count(true);
    QList<ElementTy> buf(newSize);
    for (uint32_t i = 0, j = 0; i < oldSize; ++i) {
      if (m_presenceMarks[i]) {
        buf[j++] = m_data[i];
      }
    }
    clear();
    addItems(buf);
  }

  template <Predicate1Concept<ElementTy> PredTy>
  Predicate1<uint32_t> getIndexPredicate1(PredTy predicate1) {
    return [this, predicate1](const uint32_t i0) {
      return predicate1(m_data[i0]);
    };
  }

  template <Predicate2Concept<ElementTy> PredTy>
  Predicate2<uint32_t> getIndexPredicate2(PredTy predicate2) {
    return [this, predicate2](const uint32_t i0, const uint32_t i1) {
      return predicate2(m_data[i0], m_data[i1]);
    };
  }

  QList<uint32_t>& getIndexes() { return m_indexes; }
  QList<ElementTy>& getData() { return m_data; }
  const QList<ElementTy>& getData() const { return m_data; }
  QBitArray& getPresenceMarks() { return m_presenceMarks; }

 private:
  QList<ElementTy> m_data;
  QList<uint32_t> m_indexes;
  QBitArray m_presenceMarks;
};

template <class ElementTy>
class Collection {
 public:
  class DisplayGroup : public Group {
   public:
    DisplayGroup(Group g) : Group(g) {}

   private:
    using Group::setBegin;
    using Group::setSize;
  };

  Collection()
      : m_source(),
        m_views(),
        m_finalIndexes(),
        m_finalLayout(),
        m_finalFilter(),
        m_nItemsRemoved(0),
        m_firstViewToUpdate(nViews),
        m_firstViewToReset(nViews),
        m_needShrinkData(false) {
    for (uint32_t i = 0; i < removingViewId; ++i) {
      m_views.emplace_back(new NullView(m_source.getIndexes()));
    }
    m_views.emplace_back(
        new RemovingView(m_source.getIndexes(), m_source.getPresenceMarks()));
    for (uint32_t i = removingViewId + 1; i < nViews; ++i) {
      m_views.emplace_back(new NullView(m_source.getIndexes()));
    }
  }

  const QList<ElementTy>& getData() const { return m_source.getData(); }

  uint32_t getInnerIndex(uint32_t pos) {
    if (needUpdate()) {
      update();
    }
    return m_finalIndexes[pos];
  }

  const ElementTy& getItem(uint32_t pos) {
    if (needUpdate()) {
      update();
    }
    return m_source.getData()[m_finalIndexes[pos]];
  }

  uint32_t unfilteredSize() {
    return m_source.getData().size() - m_nItemsRemoved;
  }

  uint32_t innerSize() {
    if (needUpdate()) {
      update();
    }
    return m_source.getData().size();
  }

  uint32_t filteredSize() {
    if (needUpdate()) {
      update();
    }
    return m_finalIndexes.size();
  }

  uint32_t countGroups() {
    if (needUpdate()) {
      update();
    }
    return m_finalLayout.size();
  }

  DisplayGroup getGroupByOrder(uint32_t order) {
    if (needUpdate()) {
      update();
    }
    return DisplayGroup(m_finalLayout[order]);
  }

  void clearData() {
    m_source.clear();
    m_finalIndexes.clear();
    m_finalLayout.clear();
    m_finalFilter.clear();
    m_nItemsRemoved = 0;
    m_needShrinkData = false;
    // Clear cached layouts
    m_firstViewToUpdate = 0;
    m_firstViewToReset = 0;
  }

  std::optional<uint32_t> find(const ElementTy& item) {
    if (needUpdate()) {
      update();
    }
    for (uint32_t i = 0; i < m_finalIndexes.size(); ++i) {
      if ((*m_source.getData())[m_finalIndexes[i]] == item) {
        return m_finalIndexes[i];
      }
    }
  }

  void addItems(QList<ElementTy> items) {
    m_source.addItems(items);
    m_firstViewToUpdate = 0;
  }

  void removeItemByInnerIndex(uint32_t index) {
    m_source.markRemoved(index);
    if (++m_nItemsRemoved * 2 > m_source.getData().size()) {
      m_needShrinkData = true;
    }

    m_firstViewToReset = std::min(m_firstViewToReset, removingViewId);
  }

  void removeItemAt(uint32_t pos) {
    removeItemByInnerIndex(m_finalIndexes[pos]);
  }

  void removeItem(const ElementTy& item) {
    if (auto opt = find(item); opt) {
      removeItemAt(opt.value());
    }
  }

  template <Predicate2Concept<ElementTy> PredTy1,
            Predicate2Concept<ElementTy> PredTy2>
  void setGrouping(uint32_t groupingNum,
                   PredTy1 sameGroupPred,
                   PredTy2 groupLessPred) {
    assert(groupingNum < nMaxGroupings);
    const uint32_t viewId = grouping0ViewId + groupingNum;
    const GroupLayoutBase* inputLayout = nullptr;
    if (viewId > 0) {
      inputLayout = m_views[viewId - 1]->outputLayout();
    }
    m_views[viewId].reset(
        new GroupingView(m_source.getIndexPredicate2(sameGroupPred),
                         m_source.getIndexPredicate2(groupLessPred),
                         m_source.getIndexes(), inputLayout));

    m_firstViewToReset = std::min(m_firstViewToReset, viewId);
  }

  void resetGrouping(uint32_t groupingNum) {
    assert(groupingNum < nMaxGroupings);
    const uint32_t viewId = grouping0ViewId + groupingNum;
    m_views[viewId].reset(new NullView(m_source.getIndexes()));

    m_firstViewToReset = std::min(m_firstViewToReset, viewId);
  }

  template <Predicate2Concept<ElementTy> PredTy>
  void setSorting(PredTy elementLessPred) {
    const GroupLayoutBase* inputLayout =
        m_views[sortingViewId - 1]->outputLayout();
    m_views[sortingViewId].reset(
        new SortingView(m_source.getIndexPredicate2(elementLessPred),
                        m_source.getIndexes(), inputLayout));

    m_firstViewToReset = std::min(m_firstViewToReset, sortingViewId);
  }

  void resetSorting() {
    const GroupLayoutBase* inputLayout =
        m_views[sortingViewId - 1]->outputLayout();
    m_views[sortingViewId].reset(new NullView(m_source.getIndexes()));

    m_firstViewToReset = std::min(m_firstViewToReset, sortingViewId);
  }

  template <Predicate1Concept<ElementTy> PredTy>
  void setFiltering(uint32_t filterNum, PredTy filter) {
    assert(filterNum < nMaxFilters);
    const uint32_t viewId = filtering0ViewId + filterNum;
    const GroupLayoutBase* inputLayout = m_views[viewId - 1]->outputLayout();
    m_views[viewId].reset(new FilteringView(m_source.getIndexPredicate1(filter),
                                            m_source.getIndexes()));

    m_firstViewToReset = std::min(m_firstViewToReset, viewId);
  }

  void resetFiltering(uint32_t filterNum) {
    assert(filterNum < nMaxFilters);
    const uint32_t viewId = filtering0ViewId + filterNum;
    const GroupLayoutBase* inputLayout = m_views[viewId - 1]->outputLayout();
    m_views[viewId].reset(new NullView(m_source.getIndexes()));

    m_firstViewToReset = std::min(m_firstViewToReset, viewId);
  }

  static constexpr uint32_t nMaxGroupings = 3;
  static constexpr uint32_t nMaxFilters = 3;

 private:
  static constexpr uint32_t grouping0ViewId = 0;
  static constexpr uint32_t filtering0ViewId = grouping0ViewId + nMaxGroupings;
  static constexpr uint32_t removingViewId = filtering0ViewId + nMaxFilters;
  static constexpr uint32_t sortingViewId = removingViewId + 1;
  static constexpr uint32_t nViews = sortingViewId + 1;

  void updateViews(uint32_t startId = 0) {
    for (uint32_t i = startId; i < nViews; ++i) {
      m_views[i]->update();
    }
  }

  void resetViews(uint32_t startId = 0) {
    const GroupLayoutBase* inputLayout = nullptr;
    if (startId > 0) {
      inputLayout = m_views[startId - 1]->outputLayout();
    }
    for (uint32_t i = startId; i < nViews; ++i) {
      m_views[i]->setInput(inputLayout);
      m_views[i]->reset();
      inputLayout = m_views[i]->outputLayout();
    }
  }

  void updateFinalLayout() {
    m_finalIndexes.clear();
    m_finalLayout.clear();
    m_finalFilter.clear();
    const QList<uint32_t>& indexes = m_source.getIndexes();
    const GroupLayoutBase* inputLayout = m_views[nViews - 1]->outputLayout();
    m_finalFilter.resize(indexes.size());

    if (inputLayout == nullptr) {
      m_finalIndexes = indexes;
      m_finalLayout.append(Group(0, 0, 0, indexes.size()));
      m_finalFilter.fill(true);
    } else {
      m_finalFilter.fill(false);
      for (uint32_t groupNum = 0; groupNum < inputLayout->countGroups();
           ++groupNum) {
        const Group& g = inputLayout->getByOrder(groupNum);
        for (uint32_t i = g.begin(); i < g.end(); ++i) {
          m_finalIndexes.append(indexes[i]);
          m_finalFilter[indexes[i]] = true;
        }
        const uint32_t begin = m_finalIndexes.size() - g.size();
        const uint32_t id = m_finalLayout.size();
        const uint32_t sample = m_finalIndexes[begin];
        const uint32_t size = g.size();
        m_finalLayout.append(Group(id, sample, begin, size));
      }
    }
  }

  bool needUpdate() const noexcept {
    return m_firstViewToUpdate < nViews || m_firstViewToReset < nViews;
  }

  void update() {
    if (m_needShrinkData) {
      m_source.shrinkData();
      m_nItemsRemoved = 0;
      m_firstViewToReset = 0;
      m_needShrinkData = false;
    }
    if (m_firstViewToReset < nViews) {
      resetViews(m_firstViewToReset);
      if (m_firstViewToUpdate < m_firstViewToReset) {
        updateViews(m_firstViewToUpdate);
      } else {
        updateViews(m_firstViewToReset);
      }
      updateFinalLayout();
    } else if (m_firstViewToUpdate < nViews) {
      updateViews(m_firstViewToUpdate);
      updateFinalLayout();
    }
    m_firstViewToReset = nViews;
    m_firstViewToUpdate = nViews;
  }

  IndexedSource<ElementTy> m_source;
  std::vector<std::unique_ptr<CollectionView>> m_views;
  uint32_t m_nItemsRemoved;
  QList<uint32_t> m_finalIndexes;
  QList<Group> m_finalLayout;
  QBitArray m_finalFilter;

  uint32_t m_firstViewToUpdate;
  uint32_t m_firstViewToReset;
  bool m_needShrinkData;
};
