#include "picture_collection.hpp"
#include <QTest>
#include <QtGui/qguiapplication_platform.h>
#include <cstdint>
#include <memory>
#include <qbitarray.h>
#include <qdebug.h>
#include <qnamespace.h>
#include <qtestcase.h>


struct CollectionElement
{
  int a {0};
  int b {0};
  int c {0};

  static bool groupEqual0(const CollectionElement& lhs, const CollectionElement& rhs)
  {
    return lhs.a == rhs.a;
  } 

  static bool groupLess0(const CollectionElement& lhs, const CollectionElement& rhs)
  {
    return lhs.a < rhs.a;
  }

  static bool groupEqual1(const CollectionElement& lhs, const CollectionElement& rhs)
  {
    return lhs.b == rhs.b;
  } 

  static bool groupLess1(const CollectionElement& lhs, const CollectionElement& rhs)
  {
    return lhs.b < rhs.b;
  }

  static bool groupEqual2(const CollectionElement& lhs, const CollectionElement& rhs)
  {
    return lhs.c == rhs.c;
  } 

  static bool groupLess2(const CollectionElement& lhs, const CollectionElement& rhs)
  {
    return lhs.c < rhs.c;
  }

  static bool elementLess0(const CollectionElement& lhs, const CollectionElement& rhs)
  {
    return lhs.a < rhs.a;
  }

  static bool elementLess1(const CollectionElement& lhs, const CollectionElement& rhs)
  {
    return lhs.b < rhs.b;
  }

  static bool filter0(const CollectionElement& el)
  {
    return el.a > 0;
  }

  static bool filter1(const CollectionElement& el)
  {
    return el.b > 0;
  }
};

class TestPictureCollection: public QObject
{
  Q_OBJECT
public:

private:
  enum class ViewType
  {
    null = 0,
    grouping0,
    grouping1,
    grouping2,
    filtering0,
    filtering1,
    removing0,
    removing1,
    sorting0,
    sorting1,
  };

  struct TestCaseData
  {
    TestCaseData()
    {
      QFETCH(QList<ViewType>, views);
      QFETCH(QList<QList<CollectionElement>>, input);
      QFETCH(QList<uint32_t>, groupSizes);
      QFETCH(QList<uint32_t>, order);

      this->views = std::move(views);
      this->input = std::move(input);
      this->groupSizes = std::move(groupSizes);
      this->order = std::move(order);
    }

    QList<ViewType> views;
    QList<QList<CollectionElement>> input;
    QList<uint32_t> groupSizes;
    QList<uint32_t> order;
  };

  struct ViewChain
  {
    template <class ViewTy, class ...Args>
    void addView(Args&&... args)
    {
      views.append(std::make_shared<ViewTy>(std::forward<Args>(args)...));
    }

    void update(uint32_t startView = 0)
    {
      for (uint32_t i = startView; i < views.size(); ++i) {
        views[i]->update();
      }      
    }

    const GroupLayoutBase* outputLayout() const
    {
      if (! views.empty()) {
        return views.last()->outputLayout();
      }
      return nullptr;
    }

    QList<std::shared_ptr<CollectionView>> views;
  };

  class CollectionViewTester
  {
  public:
    void test()
    {
      TestCaseData testData;
      IndexedSource<CollectionElement> source;
      QList<QBitArray> removeMarkLists;      
      QList<std::function<bool(const CollectionElement&)>> removeFunctions;
      const GroupLayoutBase* prevLayout = nullptr;
      for (ViewType vt: testData.views) {
        switch (vt) {
        case ViewType::null:
          m_views.addView<NullView>(source.getIndexes(), prevLayout);
          break;
        case ViewType::grouping0:
          m_views.addView<GroupingView>(source.getIndexPredicate2(CollectionElement::groupEqual0),
                                         source.getIndexPredicate2(CollectionElement::groupLess0),
                                         source.getIndexes(), prevLayout);
          break;
        case ViewType::grouping1:
          m_views.addView<GroupingView>(source.getIndexPredicate2(CollectionElement::groupEqual1),
                                         source.getIndexPredicate2(CollectionElement::groupLess1),
                                         source.getIndexes(), prevLayout);
          break;
        case ViewType::grouping2:
          m_views.addView<GroupingView>(source.getIndexPredicate2(CollectionElement::groupEqual2),
                                         source.getIndexPredicate2(CollectionElement::groupLess2),
                                         source.getIndexes(), prevLayout);
          break;
        case ViewType::filtering0:
          m_views.addView<FilteringView>(source.getIndexPredicate1(CollectionElement::filter0),
                                         source.getIndexes(), prevLayout);
          break;
        case ViewType::filtering1:
          m_views.addView<FilteringView>(source.getIndexPredicate1(CollectionElement::filter1),
                                         source.getIndexes(), prevLayout);
          break;
        case ViewType::removing0:
          removeMarkLists.append(QBitArray());
          m_views.addView<RemovingView>(source.getIndexes(), removeMarkLists.last(), prevLayout);
          removeFunctions.append([](const CollectionElement& el)
            {
              return el.a != 9;
            });
          break;
        case ViewType::removing1:
          removeMarkLists.append(QBitArray());
          m_views.addView<RemovingView>(source.getIndexes(), removeMarkLists.last(), prevLayout);
          removeFunctions.append([](const CollectionElement& el)
            {
              return el.b != 9;
            });
          break;
        case ViewType::sorting0:
          m_views.addView<SortingView>(source.getIndexPredicate2(CollectionElement::elementLess0),
                                        source.getIndexes(), prevLayout);
          break;
        case ViewType::sorting1:
          m_views.addView<SortingView>(source.getIndexPredicate2(CollectionElement::elementLess1),
                                        source.getIndexes(), prevLayout);
          break;
        break;
        }
        prevLayout = m_views.outputLayout();
      }
      for (const QList<CollectionElement>& portion: testData.input) {
        const uint32_t oldSize = source.getData().size();
        source.addItems(portion);
        const uint32_t newSize = source.getData().size();
        for (uint32_t funcNum = 0; funcNum < removeFunctions.size(); ++funcNum) {
          QBitArray& removeFilter = removeMarkLists[funcNum];
          removeFilter.resize(newSize);
          removeFilter.fill(true, oldSize, newSize);
          for (uint32_t i = oldSize; i < newSize; ++i) {
            removeFilter[i] = removeFilter[i] && removeFunctions[funcNum](source.getData()[i]);
          }
        }
        m_views.update();
      }
      verifyOutput(m_views.outputLayout(), source.getIndexes(), testData);
    }
  private:
    void verifyOutput(const GroupLayoutBase* outputLayout, const QList<uint32_t>& indexes, const TestCaseData& testData)
    {
      QList<uint32_t> actualSizes;
      if (outputLayout != nullptr) {
        for (uint32_t i = 0; i < outputLayout->countGroups(); ++i) {
          actualSizes.append(outputLayout->getByOrder(i).size());
        }
      }
      qDebug()<<actualSizes;
      QCOMPARE(actualSizes, testData.groupSizes);
      qDebug()<<indexes;
      QCOMPARE(indexes, testData.order);
    }
    ViewChain m_views;
  };

private slots:
  void collectionViewTests_data();
  void collectionViewTests();
  void collectionTests();
  void failedTests_data();
  void failedTests();
};

void TestPictureCollection::collectionViewTests_data()
{
  QTest::addColumn<QList<ViewType>>("views");
  QTest::addColumn<QList<QList<CollectionElement>>>("input");
  QTest::addColumn<QList<uint32_t>>("groupSizes");
  QTest::addColumn<QList<uint32_t>>("order");

  QTest::newRow("Grouping 1. One element groups") << QList{ViewType::grouping0}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{1},
                     CollectionElement{2}, CollectionElement{0}
                     }}
                     << QList<uint32_t>{1, 1, 1}
                     << QList<uint32_t>{2, 0, 1};

  QTest::newRow("Grouping 1. 1 group") << QList{ViewType::grouping0}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0},
                     CollectionElement{0}, CollectionElement{0}
                     }}
                     << QList<uint32_t>{3}
                     << QList<uint32_t>{0, 1, 2};

  QTest::newRow("Grouping 1. 2 groups 3 elements") << QList{ViewType::grouping0}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0},
                     CollectionElement{1}, CollectionElement{0},
                     CollectionElement{1}, CollectionElement{1},
                     CollectionElement{0}
                     }} 
                     << QList<uint32_t>{3, 3}
                     << QList<uint32_t>{0, 2, 5, 1, 3, 4};

  QTest::newRow("Grouping 1. 3 different groups") << QList{ViewType::grouping0}
                      << QList<QList<CollectionElement>>{{
                     CollectionElement{2},
                     CollectionElement{2}, CollectionElement{0},
                     CollectionElement{1}, CollectionElement{1},
                     CollectionElement{2}
                     }} 
                     << QList<uint32_t>{1, 2, 3}
                     << QList<uint32_t>{2, 3, 4, 0, 1, 5};

  QTest::newRow("Grouping 2. One element groups") << QList{ViewType::grouping0, ViewType::grouping1}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{1, 1},
                     CollectionElement{2, 2}, CollectionElement{0, 0}
                     }}
                     << QList<uint32_t>{1, 1, 1}
                     << QList<uint32_t>{2, 0, 1};

  QTest::newRow("Grouping 2. 1 group") << QList{ViewType::grouping0, ViewType::grouping1}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0, 0},
                     CollectionElement{0, 0}, CollectionElement{0, 0}
                     }} 
                     << QList<uint32_t>{3}
                     << QList<uint32_t>{0, 1, 2};

  QTest::newRow("Grouping 2. 2 groups 2 subgroups") << QList{ViewType::grouping0, ViewType::grouping1}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0, 0},
                     CollectionElement{0, 1}, CollectionElement{1, 0},
                     CollectionElement{1, 1}
                     }}
                     << QList<uint32_t>{1, 1, 1, 1}
                     << QList<uint32_t>{0, 1, 2, 3};

  QTest::newRow("Grouping 2. 3 different groups") << QList{ViewType::grouping0, ViewType::grouping1}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{2, 0},
                     CollectionElement{2, 2}, CollectionElement{0, 2},
                     CollectionElement{1, 1}, CollectionElement{1, 0},
                     CollectionElement{2, 2}
                     }} 
                     << QList<uint32_t>{1, 1, 1, 1, 2}
                     << QList<uint32_t>{2, 4, 3, 0, 1, 5};

  QTest::newRow("Grouping 3. 8 one element subgroups") << QList{ViewType::grouping0, ViewType::grouping1, ViewType::grouping2}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0, 0, 1}, CollectionElement{0, 0, 0},
                     CollectionElement{1, 1, 1}, CollectionElement{1, 1, 0}, CollectionElement{1, 0, 1},
                     CollectionElement{0, 1, 1}, CollectionElement{0, 1, 0}, CollectionElement{1, 0, 0}
                     }}
                     << QList<uint32_t>{1, 1, 1, 1, 1, 1, 1, 1}
                     << QList<uint32_t>{1, 0, 6, 5, 7, 4, 3, 2};

  QTest::newRow("Grouping 3. 4 different subgroups") << QList{ViewType::grouping0, ViewType::grouping1, ViewType::grouping2}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0, 0, 0},
                     CollectionElement{1, 0, 0}, CollectionElement{0, 1, 0},
                     CollectionElement{1, 0, 1}, CollectionElement{0, 0, 0},
                     CollectionElement{0, 1, 0}, CollectionElement{0, 0, 0},
                     CollectionElement{1, 0, 1}
                     }}
                     << QList<uint32_t>{3, 2, 1, 2}
                     << QList<uint32_t>{0, 4, 6, 2, 5, 1, 3, 7};

  QTest::newRow("Grouping after expanded. 8 one element subgroups") << QList{ViewType::grouping0, ViewType::grouping1, ViewType::grouping2}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0, 0, 1}, CollectionElement{0, 0, 0},
                     CollectionElement{1, 1, 1}, CollectionElement{1, 1, 0}
                     }, {
                     CollectionElement{1, 0, 1}
                     }, {
                     CollectionElement{0, 1, 1}, CollectionElement{0, 1, 0},
                     CollectionElement{1, 0, 0}
                     }}
                     << QList<uint32_t>{1, 1, 1, 1, 1, 1, 1, 1}
                     << QList<uint32_t>{1, 0, 6, 5, 7, 4, 3, 2};

  QTest::newRow("Grouping after expanded. 4 different subgroups") << QList{ViewType::grouping0, ViewType::grouping1, ViewType::grouping2}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0, 0, 0}
                     }, {
                     CollectionElement{1, 0, 0}, CollectionElement{0, 1, 0}
                     }, {
                     CollectionElement{1, 0, 1}, CollectionElement{0, 0, 0},
                     CollectionElement{0, 1, 0}, CollectionElement{0, 0, 0},
                     CollectionElement{1, 0, 1}
                     }} 
                     << QList<uint32_t>{3, 2, 1, 2}
                     << QList<uint32_t>{0, 4, 6, 2, 5, 1, 3, 7};

  QTest::newRow("Empty sorting") << QList{ViewType::sorting0}
                     << QList<QList<CollectionElement>>{{}}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{};

  QTest::newRow("Sorting 1. One element") << QList{ViewType::sorting0}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{0};

  QTest::newRow("Sorting 1. Ordered collection") << QList{ViewType::sorting0}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0}, CollectionElement{1},
                     CollectionElement{1},
                               CollectionElement{2}, CollectionElement{2},
                               CollectionElement{2}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{0, 1, 2, 3, 4, 5};

  QTest::newRow("Sorting 1. Semi-ordered collection") << QList{ViewType::sorting0}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0},
                               CollectionElement{1}, CollectionElement{2},
                               CollectionElement{0}, CollectionElement{1},
                               CollectionElement{2}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{0, 3, 1, 4, 2, 5};

  QTest::newRow("Sorting 1. Reverse ordered collection") << QList{ViewType::sorting0}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{2},
                      CollectionElement{1}, CollectionElement{0}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{2, 1, 0};

  QTest::newRow("Sorting 1. More elements") << QList{ViewType::sorting0}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{7},
                                CollectionElement{0}, CollectionElement{1},
                                CollectionElement{2}, CollectionElement{3},
                                CollectionElement{0}, CollectionElement{7},
                                CollectionElement{6}, CollectionElement{4},
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{1, 5, 2, 3, 4, 8, 7, 0, 6};

  QTest::newRow("Sorting 2. One element") << QList{ViewType::sorting0, ViewType::sorting1}
                     << QList<QList<CollectionElement>>{{
                      CollectionElement{0, 0}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{0};

  QTest::newRow("Sorting 2. Ordered collection") << QList{ViewType::sorting0, ViewType::sorting1}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0, 0},
                               CollectionElement{0, 1}, CollectionElement{0, 1},
                               CollectionElement{0, 2}, CollectionElement{0, 2},
                               CollectionElement{0, 2}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{0, 1, 2, 3, 4, 5};

  QTest::newRow("Sorting 2. Reverse ordered collection") << QList{ViewType::sorting0, ViewType::sorting1}
                     << QList<QList<CollectionElement>>{{
                                CollectionElement{2, 0},
                                CollectionElement{1, 1}, CollectionElement{0, 2}
                     }}
                               << QList<uint32_t>{}
                               << QList<uint32_t>{0, 1, 2};

  QTest::newRow("Sorting 2. More elements") << QList{ViewType::sorting0, ViewType::sorting1}
                     << QList<QList<CollectionElement>>{{
                               CollectionElement{0, 4},
                                CollectionElement{0, 1}, CollectionElement{1, 2},
                                CollectionElement{1, 3}, CollectionElement{0, 0},
                                CollectionElement{0, 3}, CollectionElement{0, 1},
                                CollectionElement{1, 1}, CollectionElement{0, 0},
                     }}
                               << QList<uint32_t>{}
                               << QList<uint32_t>{4, 8, 1, 6, 7, 2, 5, 3, 0};

  QTest::newRow("Sorting after expanded. 1 + 1 element")  << QList{ViewType::sorting0}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{1}
                     }, {
                     CollectionElement{0}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{1, 0};

  QTest::newRow("Sorting after expanded. Ordered collection")  << QList{ViewType::sorting0}
                     << QList<QList<CollectionElement>>{{
                               CollectionElement{0},
                               CollectionElement{1}, CollectionElement{1},
                               CollectionElement{2}
                     }, {
                        CollectionElement{2},
                       CollectionElement{3}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{0, 1, 2, 3, 4, 5};

  QTest::newRow("Sorting after expanded. More elements")  << QList{ViewType::sorting0}
                     << QList<QList<CollectionElement>>{{
                               CollectionElement{4},
                                CollectionElement{1}, CollectionElement{2},
                                CollectionElement{3}
                     }, {
                       CollectionElement{0},
                                CollectionElement{3}, CollectionElement{1},
                                CollectionElement{1}, CollectionElement{0}
                     }}
                     << QList<uint32_t>{}
                                << QList<uint32_t>{4, 8, 1, 6, 7, 2, 3, 5, 0};

  QTest::newRow("Sorting grouped. One element") << QList{ViewType::grouping0, ViewType::sorting1}
                     << QList<QList<CollectionElement>>{{
                      CollectionElement{0, 0}
                     }}
                     << QList<uint32_t>{1}
                               << QList<uint32_t>{0};

  QTest::newRow("Sorting grouped. Ordered collection") << QList{ViewType::grouping0, ViewType::sorting1}
                     << QList<QList<CollectionElement>>{{
                               CollectionElement{0, 0},
                               CollectionElement{1, 0}, CollectionElement{1, 1},
                               CollectionElement{2, 0}, CollectionElement{2, 1},
                               CollectionElement{2, 2}
                     }}
                     << QList<uint32_t>{1, 2, 3}
                               << QList<uint32_t>{0, 1, 2, 3, 4, 5};

  QTest::newRow("Sorting grouped. Semi-ordered collection") << QList{ViewType::grouping0, ViewType::sorting1}
                     << QList<QList<CollectionElement>>{{
                               CollectionElement{1, 0},
                               CollectionElement{1, 1}, CollectionElement{1, 2},
                               CollectionElement{0, 0}, CollectionElement{0, 1},
                               CollectionElement{0, 2}
                     }}
                     << QList<uint32_t>{3, 3}
                     << QList<uint32_t>{3, 4, 5, 0, 1, 2};

  QTest::newRow("Sorting grouped. More elements") << QList{ViewType::grouping0, ViewType::sorting1}
                     << QList<QList<CollectionElement>>{{
                                CollectionElement{0, 3},
                                CollectionElement{1, 2}, CollectionElement{0, 1},
                                CollectionElement{1, 5}, CollectionElement{0, 0},
                                CollectionElement{1, 4}, CollectionElement{0, 8},
                                CollectionElement{1, 6}, CollectionElement{0, 6},
                        }}
                     << QList<uint32_t>{5, 4}
                     << QList<uint32_t>{4, 2, 0, 8, 6, 1, 5, 3, 7};

  QTest::newRow("Sorting grouped after expanded. 0 + 1 element") << QList{ViewType::grouping0, ViewType::sorting1}
                     << QList<QList<CollectionElement>>{{
                     }, {
                     CollectionElement{0, 0}
                     }}
                     << QList<uint32_t>{1}
                     << QList<uint32_t>{0};

  QTest::newRow("Sorting grouped after expanded. Ordered collection") << QList{ViewType::grouping0, ViewType::sorting1}
                     << QList<QList<CollectionElement>>{{
                                  CollectionElement{0, 0},
                               CollectionElement{1, 0}, CollectionElement{1, 1},
                               CollectionElement{2, 0}
                     }, {
                       CollectionElement{2, 1},
                               CollectionElement{2, 2}
                               }}
                     << QList<uint32_t>{1, 2, 3}
                     << QList<uint32_t>{0, 1, 2, 3, 4, 5};

  QTest::newRow("Sorting grouped after expanded. More elements") << QList{ViewType::grouping0, ViewType::sorting1}
                     << QList<QList<CollectionElement>>{{
                                CollectionElement{0, 3},
                                CollectionElement{1, 2}, CollectionElement{0, 1},
                                CollectionElement{1, 5}, CollectionElement{0, 0}
                     }, {
                               CollectionElement{1, 4}, CollectionElement{0, 8},
                                CollectionElement{1, 6}, CollectionElement{0, 6}
                                }}
                     << QList<uint32_t>{5, 4}
                     << QList<uint32_t>{4, 2, 0, 8, 6, 1, 5, 3, 7};

  QTest::newRow("Empty filtering") << QList{ViewType::filtering0}
                     << QList<QList<CollectionElement>>{{}}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{};

  QTest::newRow("Filtering 1. One element") << QList{ViewType::filtering0}
                     << QList<QList<CollectionElement>>{{
                      CollectionElement{0}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{0};

  QTest::newRow("Filtering 1. Many elements") << QList{ViewType::filtering0}
                     << QList<QList<CollectionElement>>{{
                       CollectionElement{0, 0},
                               CollectionElement{0, 1}, CollectionElement{1, 0},
                               CollectionElement{1, 1}, CollectionElement{0, 3},
                               CollectionElement{3, 0}
                     }}
                     << QList<uint32_t>{3}
                     << QList<uint32_t>{2, 3, 5, 0, 1, 4};

  QTest::newRow("Filtering 2. One element") << QList{ViewType::filtering0, ViewType::filtering1}
                     << QList<QList<CollectionElement>>{{
                      CollectionElement{0, 1}
                      }}
                               << QList<uint32_t>{}
                               << QList<uint32_t>{0};

  QTest::newRow("Filtering 2. Many elements") << QList{ViewType::filtering0, ViewType::filtering1}
                     << QList<QList<CollectionElement>>{{
                                CollectionElement{0, 0},
                               CollectionElement{0, 1}, CollectionElement{1, 0},
                               CollectionElement{1, 1}, CollectionElement{0, 3},
                               CollectionElement{3, 0}
                               }}
                               << QList<uint32_t>{1}
                               << QList<uint32_t>{3, 2, 5, 0, 1, 4};

  QTest::newRow("Filtering after expanded. 0 + 1 element") << QList{ViewType::filtering0, ViewType::filtering1}
                     << QList<QList<CollectionElement>>{{
                     }, {
                     CollectionElement{0, 1}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{0};

  QTest::newRow("Filtering after expanded. Many elements") << QList{ViewType::filtering0, ViewType::filtering1}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{0, 0},
                     CollectionElement{0, 1}, CollectionElement{1, 0}
                     }, {
                     CollectionElement{1, 1}, CollectionElement{0, 3},
                     CollectionElement{3, 0}
                     }}
                     << QList<uint32_t>{1}
                     << QList<uint32_t>{3, 2, 5, 0, 1, 4};

  QTest::newRow("Filtering after expanded. Many elements 2") << QList{ViewType::filtering0, ViewType::filtering1}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{1, 0},
                     CollectionElement{1, 1}, CollectionElement{0, 0},
                     CollectionElement{0, 1}
                     }, {
                     CollectionElement{1, 1},
                     CollectionElement{0, 1}, CollectionElement{0, 0},
                     CollectionElement{1, 0}, CollectionElement{1, 1}
                     }}
                     << QList<uint32_t>{3}
                     << QList<uint32_t>{1, 4, 8, 0, 7, 2, 3, 5, 6};

  QTest::newRow("Empty removing") << QList{ViewType::removing0}
                     << QList<QList<CollectionElement>>{{}}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{};

  QTest::newRow("Removing 1. One element") << QList{ViewType::removing0}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{9}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{0};

  QTest::newRow("Removing 1. Many elements") << QList{ViewType::removing0}
                     << QList<QList<CollectionElement>>{{
                     CollectionElement{9, 9},
                     CollectionElement{9, 1}, CollectionElement{1, 9},
                     CollectionElement{1, 1}, CollectionElement{9, 3},
                     CollectionElement{3, 9}
                     }}
                     << QList<uint32_t>{3}
                     << QList<uint32_t>{2, 3, 5, 0, 1, 4};

  QTest::newRow("Removing after expanded. 0 + 1 element") << QList{ViewType::removing0}
                     << QList<QList<CollectionElement>>{{
                     }, {
                     CollectionElement{9}
                     }}
                     << QList<uint32_t>{}
                     << QList<uint32_t>{0};

  QTest::newRow("Removing after expanded. Many elements") << QList{ViewType::removing0}
                    << QList<QList<CollectionElement>>{{
                    CollectionElement{9, 9},
                    CollectionElement{9, 1}, CollectionElement{1, 9}
                    }, {
                    CollectionElement{1, 1}, CollectionElement{9, 3},
                    CollectionElement{3, 9}
                    }}
                    << QList<uint32_t>{3}
                    << QList<uint32_t>{2, 3, 5, 0, 1, 4};

  QTest::newRow("Removing filtering 2. Many elements") << QList{ViewType::removing0, ViewType::filtering0, ViewType::removing1, ViewType::filtering1}
                    << QList<QList<CollectionElement>>{{
                    CollectionElement{0, 0},
                    CollectionElement{0, 1}, CollectionElement{1, 0},
                    CollectionElement{0, 9}, CollectionElement{9, 0},
                    CollectionElement{1, 1}, CollectionElement{0, 3},
                    CollectionElement{2, 2}, CollectionElement{9, 9},
                   CollectionElement{9, 1}, CollectionElement{1, 9},
                   CollectionElement{3, 0}
                   }}
                   << QList<uint32_t>{2}
                   << QList<uint32_t>{5, 7, 2, 11, 10, 0, 1, 3, 6, 4, 8, 9};
  QTest::newRow("Filtering goruped. One element groups") << QList{ViewType::grouping0, ViewType::filtering1}
                   << QList<QList<CollectionElement>>{{
                   CollectionElement{1, 0},
                    CollectionElement{2, 1}, CollectionElement{0, 1}
                    }}
                    << QList<uint32_t>{1, 1}
                    << QList<uint32_t>{2, 0, 1};

  QTest::newRow("Filtering goruped. 3 different groups") << QList{ViewType::grouping0, ViewType::filtering1}
                    << QList<QList<CollectionElement>>{{
                    CollectionElement{2, 0},
                    CollectionElement{2, 1}, CollectionElement{0, 1},
                    CollectionElement{1, 1}, CollectionElement{1, 0},
                    CollectionElement{2, 1}, CollectionElement{2, 0},
                    CollectionElement{1, 0}, CollectionElement{0, 0},
                    CollectionElement{1, 1}, CollectionElement{0, 1},
                    }} 
                    << QList<uint32_t>{2, 2, 2}
                    << QList<uint32_t>{2, 10, 8,  3, 9, 4, 7,  1, 5, 0, 6};

  QTest::newRow("Filtering->Grouping->Filtering. One element groups") << QList{ViewType::grouping0, ViewType::filtering1, ViewType::grouping2}
                    << QList<QList<CollectionElement>>{{
                    CollectionElement{1, 0, 1},
                    CollectionElement{2, 1, 2}, CollectionElement{0, 1, 0}
                    }}
                    << QList<uint32_t>{1, 1}
                    << QList<uint32_t>{2, 0, 1};

  QTest::newRow("Filtering->Grouping->Filtering. 3 different groups") << QList{ViewType::grouping0, ViewType::filtering1, ViewType::grouping2}
                    << QList<QList<CollectionElement>>{{
                    CollectionElement{2, 0, 0},
                    CollectionElement{2, 1, 1}, CollectionElement{0, 1, 0},
                    CollectionElement{1, 1, 0}, CollectionElement{1, 0, 1},
                    CollectionElement{2, 1, 0}, CollectionElement{2, 0, 1},
                    CollectionElement{1, 0, 1}, CollectionElement{0, 0, 0},
                    CollectionElement{1, 1, 0}, CollectionElement{0, 1, 1},
                    }} 
                    << QList<uint32_t>{1, 1, 2, 1, 1}
                    << QList<uint32_t>{2, 10, 8,  3, 9, 4, 7,  5, 1, 0, 6};
}


void TestPictureCollection::collectionViewTests()
{
  CollectionViewTester t;  
  t.test();
}

void TestPictureCollection::collectionTests()
{
  QList<CollectionElement> items = {
    {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {1, 0, 0}, {1, 0, 1}, {1, 1, 0},
    {1, 1, 1}, {0, 0, 2}, {0, 2, 0}, {2, 0, 0}, {2, 2, 2}, {0, 1, 2}, {2, 1, 0}
  };

  QList<uint32_t> groupSizes = {4, 2, 2};
  QList<uint32_t> order = {2, 3, 12, 9, 6, 7, 13, 11};

  Collection<CollectionElement> cll; 
  cll.setGrouping(0, CollectionElement::groupEqual0, CollectionElement::groupLess0);
  cll.setFiltering(0, CollectionElement::filter1);
  cll.setSorting(CollectionElement::elementLess1);
  cll.addItems(items);
  QCOMPARE(cll.filteredSize(), 8);
  QCOMPARE(cll.countGroups(), groupSizes.size());
  for (uint32_t groupNum = 0; groupNum < cll.countGroups(); ++groupNum) {
    Collection<CollectionElement>::DisplayGroup g = cll.getGroupByOrder(groupNum);
    QCOMPARE(g.size(), groupSizes[groupNum]);
    for (uint32_t i = g.begin(); i < g.end(); ++i) {
      QCOMPARE(cll.getInnerIndex(i), order[i]);
    }
  }

  cll.resetGrouping(0);
  cll.resetFiltering(0);
  cll.resetSorting();

  QCOMPARE(cll.filteredSize(), items.size());
  const uint32_t shrinkedItems = (items.size() * 2) / 3;
  for (uint32_t i = 0; i < shrinkedItems; ++i) {
    cll.removeItemAt(i);
  }
  QCOMPARE(cll.innerSize(), items.size() - shrinkedItems);
  QCOMPARE(cll.filteredSize(), items.size() - shrinkedItems);

  const uint32_t shrinkedItems2 = (cll.innerSize() * 2) / 3;
  for (uint32_t i = 0; i < shrinkedItems2; ++i) {
    cll.removeItemAt(i);
  }
  QCOMPARE(cll.innerSize(), items.size() - shrinkedItems - shrinkedItems2);

  cll.setGrouping(1, CollectionElement::groupEqual1, CollectionElement::groupLess1);
  cll.addItems({{1, 2, 3}, {2, 3, 4}, {3, 4, 5}});
  QCOMPARE(cll.countGroups(), 4);
}

void TestPictureCollection::failedTests_data()
{
  // QTest::addColumn<QList<ViewType>>("views");
  // QTest::addColumn<QList<QList<CollectionElement>>>("input");
  // QTest::addColumn<QList<uint32_t>>("groupSizes");
  // QTest::addColumn<QList<uint32_t>>("order");
  
}

void TestPictureCollection::failedTests()
{
  // Tester t;  
  // t.test();
}

QTEST_MAIN(TestPictureCollection)
#include "test_collection.moc"
