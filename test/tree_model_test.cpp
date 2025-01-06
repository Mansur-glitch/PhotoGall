// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright © 2025 Mansur Mukhametzyanov
#include "tree_model.hpp"

#include <QDir>
#include <QTest>
#include <QAbstractItemModelTester>

class TestTreeModel: public QObject
{
  Q_OBJECT
public:

private:
struct TestDirNode
{
  TestDirNode(QString name, QList<TestDirNode> children = {})
  : m_name(name), m_children(children), m_needDeletion(false), m_path()
  {}

  ~TestDirNode()
  {
    if (m_needDeletion) {
      removeStructure();
    }
  }

  QString getPath()
  {
    return m_path;
  }

  void createSrtucture()
  {
    m_path = QCoreApplication::applicationDirPath();
    QDir testDir(m_path);
    qDebug()<<m_path;
    QVERIFY(testDir.mkdir(m_name));
    QVERIFY(testDir.cd(m_name));
    m_path = testDir.absolutePath();
    m_needDeletion = true;

    if (m_children.size() > 0) {
      createDirectoriesRecursive(testDir);
    }
  }

  void removeStructure()
  {
    QString exePath = QCoreApplication::applicationDirPath();
    QDir testDir(exePath);
    QVERIFY(testDir.cd(m_name));
    QVERIFY(testDir.removeRecursively());
    m_needDeletion = false;
  }

  TestDirNode* getChild(QString name)
  {
    auto iter = std::find_if(m_children.begin(), m_children.end(),
                             [name](const TestDirNode& n){return n.m_name == name;});
    if (iter == m_children.end()) {
      return nullptr;
    }
    return &(*iter);
  }

  void removeChild(QString name)
  {
    m_children.removeIf([name](const TestDirNode& n){return n.m_name == name;});
    QDir qdir = m_path + '/' + name;
    QVERIFY(qdir.removeRecursively());
  }

  void addChild(QString name, QList<TestDirNode> children = {})
  {
    m_children.append(TestDirNode(name, children));
    m_children.last().m_path = m_path + '/' + name;
    qDebug()<<m_path<<name;
    QVERIFY(QDir(m_path).mkdir(name));
  }

private:
  QString m_name;
  QList<TestDirNode> m_children;
  bool m_needDeletion;
  QString m_path;

  void createDirectoriesRecursive(QDir& dir)
  {
    for (TestDirNode& n: m_children) {
      QVERIFY(dir.mkdir(n.m_name));
      n.m_path = dir.absolutePath() + '/' + n.m_name;
      if (n.m_children.size() > 0) {
        dir.cd(n.m_name);
        n.createDirectoriesRecursive(dir);
      }
    }
    dir.cdUp();
  }
};

private slots:
  void emptyModel();
  void simpleHierarchy();
  void multiLayeredHierarchy();
  void addingDirectories();
  void removingDirectories();
  void goUpperDirectory();
  void goDownDirectory();
};

void TestTreeModel::emptyModel()
{
  TestDirNode directoriesStructure = {"deleteThis"};
  directoriesStructure.createSrtucture();
  DirectoryTreeModel model;
  QAbstractItemModelTester modelTester(&model);
  model.setTreeRoot(directoriesStructure.getPath());
}

void TestTreeModel::simpleHierarchy()
{
  TestDirNode directoriesStructure = {"deleteThis",
  {{"0"}, {"1"}, {"2"}, {"3"}}};
  directoriesStructure.createSrtucture();
  DirectoryTreeModel model;
  QAbstractItemModelTester modelTester(&model);
  model.setTreeRoot(directoriesStructure.getPath());
}

void TestTreeModel::multiLayeredHierarchy()
{
  TestDirNode directoriesStructure = {"deleteThis",
  {{"0", {{"00"}, {"01"}} },
   {"1", {{"10", {{"100"}} }} },
   {"2"},
   {"3", {{"30"}, {"31"}, {"32"}, {"33"}}} }};
  directoriesStructure.createSrtucture();
  DirectoryTreeModel model;
  QAbstractItemModelTester modelTester(&model);
  model.setTreeRoot(directoriesStructure.getPath());
  model.expandChildAtIndex(model.index(0,0,{}));
  model.expandChildAtIndex(model.index(1,0,{}));
  model.expandChildAtIndex(model.index(2,0,{}));
  model.expandChildAtIndex(model.index(3,0,{}));
  model.expandChildAtIndex(model.index(0, 0, model.index(1,0,{})));
}

void TestTreeModel::addingDirectories()
{
  TestDirNode directoriesStructure = {"deleteThis",
  {{"0"},
   {"1", {{"10"}} },
   {"2"},
   {"3", {{"31"}, {"33"}}} }};
  directoriesStructure.createSrtucture();
  DirectoryTreeModel model;
  QAbstractItemModelTester modelTester(&model);
  model.setTreeRoot(directoriesStructure.getPath());
  model.expandChildAtIndex(model.index(1,0,{}));
  model.expandChildAtIndex(model.index(2,0,{}));
  model.expandChildAtIndex(model.index(3,0,{}));

  TestDirNode* dir0 = directoriesStructure.getChild("0");
  dir0->addChild("00");
  dir0->addChild("01");
  model.expandChildAtIndex(model.index(0,0,{}));
  TestDirNode* dir1 = directoriesStructure.getChild("1");
  TestDirNode* dir10 = dir1->getChild("10");
  dir10->addChild("100");
  model.expandChildAtIndex(model.index(0, 0, model.index(1,0,{})));
  TestDirNode* dir3 = directoriesStructure.getChild("3");
  dir3->addChild("30");
  dir3->addChild("32");
  model.expandChildAtIndex(model.index(3,0,{}));

  QCOMPARE(model.index(0,0,model.index(3,0,{})).data(), "30");
  QCOMPARE(model.index(1,0,model.index(3,0,{})).data(), "31");
  QCOMPARE(model.index(2,0,model.index(3,0,{})).data(), "32");
  QCOMPARE(model.index(3,0,model.index(3,0,{})).data(), "33");
}

void TestTreeModel::removingDirectories()
{
  TestDirNode directoriesStructure = {"deleteThis",
  {{"0", {{"00"}, {"01"}} },
   {"1", {{"10", {{"100"}} }} },
   {"2"},
   {"3", {{"30"}, {"31"}, {"32"}, {"33"}}} }};
  directoriesStructure.createSrtucture();
  DirectoryTreeModel model;
  QAbstractItemModelTester modelTester(&model);
  model.setTreeRoot(directoriesStructure.getPath());
  model.expandChildAtIndex(model.index(0,0,{}));
  model.expandChildAtIndex(model.index(1,0,{}));
  model.expandChildAtIndex(model.index(2,0,{}));
  model.expandChildAtIndex(model.index(3,0,{}));
  model.expandChildAtIndex(model.index(0, 0, model.index(1,0,{})));

  TestDirNode* dir0 = directoriesStructure.getChild("0");
  dir0->removeChild("00");
  dir0->removeChild("01");
  model.expandChildAtIndex(model.index(0,0,{}));
  directoriesStructure.removeChild("1");
  directoriesStructure.removeChild("2");
  model.expandChildAtIndex({});
  TestDirNode* dir3 = directoriesStructure.getChild("3");
  dir3->removeChild("30");
  dir3->removeChild("32");
  model.expandChildAtIndex(model.index(1,0,{}));

  QCOMPARE(model.index(1,0,{}).data(), "3");
  QCOMPARE(model.index(0,0,model.index(1,0,{})).data(), "31");
  QCOMPARE(model.index(1,0,model.index(1,0,{})).data(), "33");
}

void TestTreeModel::goUpperDirectory()
{
  TestDirNode directoriesStructure = {"deleteThis",
  {{"0", {{"00"}, {"01"}} },
   {"1", {{"10", {{"100"}} }} },
   {"2"},
   {"3", {{"30"}, {"31"}, {"32"}, {"33"}}} }};
  directoriesStructure.createSrtucture();
  TestDirNode* firstRoot = directoriesStructure.getChild("1")->getChild("10");
  DirectoryTreeModel model;
  QAbstractItemModelTester modelTester0(&model);
  model.setTreeRoot(firstRoot->getPath());
  model.expandChildAtIndex(model.index(0,0,{}));
  model.setUpperRoot();

  QCOMPARE(model.index(0,0,{}).data(), "10");
  QCOMPARE(model.index(0,0,model.index(0,0,{})).data(), "100");

  model.setUpperRoot();
  model.expandChildAtIndex(model.index(0,0,{}));
  QCOMPARE(model.index(0,0,{}).data(), "0");
  QCOMPARE(model.index(1,0,{}).data(), "1");
  model.expandChildAtIndex(model.index(2,0,{}));
  QCOMPARE(model.index(2,0,{}).data(), "2");
  model.expandChildAtIndex(model.index(3,0,{}));
  QCOMPARE(model.index(3,0,{}).data(), "3");
  QCOMPARE(model.index(0,0,model.index(3,0,{})).data(), "30");
}

void TestTreeModel::goDownDirectory()
{
  TestDirNode directoriesStructure = {"deleteThis",
  {{"0", {{"00"}, {"01"}} },
   {"1", {{"10", {{"100"}} }} },
   {"2"},
   {"3", {{"30"}, {"31"}, {"32"}, {"33"}}} }};
  directoriesStructure.createSrtucture();
  DirectoryTreeModel model;
  QAbstractItemModelTester modelTester(&model);

  model.setTreeRoot(directoriesStructure.getPath());
  model.expandChildAtIndex(model.index(0,0,{}));
  model.expandChildAtIndex(model.index(1,0,{}));
  model.expandChildAtIndex(model.index(2,0,{}));
  model.expandChildAtIndex(model.index(3,0,{}));
  model.expandChildAtIndex(model.index(0, 0, model.index(1,0,{})));

  model.downRootTo(model.index(1, 0, {}));
  QCOMPARE(model.index(0,0,{}).data(), "10");
  QCOMPARE(model.index(0,0,model.index(0,0,{})).data(), "100");
}

QTEST_MAIN(TestTreeModel)
#include "tree_model_test.moc"
