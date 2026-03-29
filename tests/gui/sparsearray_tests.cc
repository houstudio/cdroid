#include <gtest/gtest.h>
#include <memory>
#include <core/sparsearray.h>

class SPARSEARRAY : public ::testing::Test {
protected:
    void SetUp() override {
        array.clear();
    }

    void TearDown() override {
        array.clear();
    }

    cdroid::SparseArray<int> array;
};

// 测试默认构造和 size
TEST_F(SPARSEARRAY, DefaultConstructionAndSize) {
    EXPECT_EQ(array.size(), 0U);
}

// 测试 put 和 get
TEST_F(SPARSEARRAY, PutAndGet) {
    array.put(10, 100);
    array.put(5, 50);
    array.put(20, 200);

    EXPECT_EQ(array.size(), 3U);
    EXPECT_EQ(array.get(10), 100);
    EXPECT_EQ(array.get(5), 50);
    EXPECT_EQ(array.get(20), 200);

    // 测试获取不存在的 key
    EXPECT_EQ(array.get(99), 0); // 默认值
    EXPECT_EQ(array.get(99, -1), -1); // 指定默认值
}

// 测试 key 更新
TEST_F(SPARSEARRAY, UpdateExistingKey) {
    array.put(1, 10);
    EXPECT_EQ(array.get(1), 10);

    array.put(1, 20); // 更新同一个 key
    EXPECT_EQ(array.get(1), 20);
    EXPECT_EQ(array.size(), 1U); // size 不变
}

// 测试 indexOfKey
TEST_F(SPARSEARRAY, IndexOfKey) {
    array.put(30, 300);
    array.put(10, 100);
    array.put(20, 200); // 此时 keys 应为 [10, 20, 30]

    EXPECT_EQ(array.indexOfKey(10), 0);
    EXPECT_EQ(array.indexOfKey(20), 1);
    EXPECT_EQ(array.indexOfKey(30), 2);
    EXPECT_EQ(array.indexOfKey(99), -1); // 不存在的 key
}

// 测试 indexOfValue
TEST_F(SPARSEARRAY, IndexOfValue) {
    array.put(1, 100);
    array.put(2, 200);
    array.put(3, 100); // 重复的 value

    EXPECT_EQ(array.indexOfValue(100), 0); // 返回第一次出现的索引
    EXPECT_EQ(array.indexOfValue(200), 1);
    EXPECT_EQ(array.indexOfValue(999), -1); // 不存在的 value
}

// 测试 keyAt 和 valueAt
TEST_F(SPARSEARRAY, KeyAtAndValueAt) {
    array.put(100, 1);
    array.put(50, 2);
    array.put(75, 3); // keys 应为 [50, 75, 100]

    EXPECT_EQ(array.keyAt(0), 50);
    EXPECT_EQ(array.keyAt(1), 75);
    EXPECT_EQ(array.keyAt(2), 100);

    EXPECT_EQ(array.valueAt(0), 2);
    EXPECT_EQ(array.valueAt(1), 3);
    EXPECT_EQ(array.valueAt(2), 1);
}

// 测试 remove
TEST_F(SPARSEARRAY, Remove) {
    array.put(1, 10);
    array.put(2, 20);
    array.put(3, 30);

    array.remove(2); // 移除存在的 key
    EXPECT_EQ(array.size(), 2U);
    EXPECT_EQ(array.get(2, -1), -1); // 确认已移除

    array.remove(99); // 移除不存在的 key
    EXPECT_EQ(array.size(), 2U); // size 不变

    EXPECT_EQ(array.get(1), 10);
    EXPECT_EQ(array.get(3), 30);
}

// 测试 removeAt
TEST_F(SPARSEARRAY, RemoveAt) {
    array.put(1, 10);
    array.put(2, 20);
    array.put(3, 30); // [1->10, 2->20, 3->30]

    array.removeAt(1); // 移除索引 1 的元素 (2->20)
    EXPECT_EQ(array.size(), 2U);

    // 由于移除后元素会前移，keys 现在是 [1, 3]
    EXPECT_EQ(array.keyAt(0), 1);
    EXPECT_EQ(array.valueAt(0), 10);
    EXPECT_EQ(array.keyAt(1), 3);
    EXPECT_EQ(array.valueAt(1), 30);
}

// 测试 clear
TEST_F(SPARSEARRAY, Clear) {
    array.put(1, 10);
    array.put(2, 20);
    EXPECT_EQ(array.size(), 2U);

    array.clear();
    EXPECT_EQ(array.size(), 0U);
    EXPECT_EQ(array.get(1, -1), -1);
    EXPECT_EQ(array.get(2, -1), -1);
}

// 测试 append
TEST_F(SPARSEARRAY, Append) {
    array.append(1, 100); // 直接追加
    array.append(2, 200); // 直接追加
    array.append(3, 300); // 直接追加

    EXPECT_EQ(array.size(), 3U);
    EXPECT_EQ(array.keyAt(0), 1);
    EXPECT_EQ(array.keyAt(1), 2);
    EXPECT_EQ(array.keyAt(2), 3);

    // 尝试追加一个较小的 key，应该调用 put 逻辑
    array.append(0, 400);
    EXPECT_EQ(array.size(), 4U);
    // 0 应该排在最前面，keys 现在是 [0, 1, 2, 3]
    EXPECT_EQ(array.keyAt(0), 0);
    EXPECT_EQ(array.valueAt(0), 400);
}

// 测试模板泛型 (使用 std::string 作为 value)
TEST_F(SPARSEARRAY, GenericTemplate) {
    cdroid::SparseArray<std::string> stringArray;
    stringArray.put(1, "Hello");
    stringArray.put(2, "World");

    EXPECT_EQ(stringArray.size(), 2U);
    EXPECT_EQ(stringArray.get(1), "Hello");
    EXPECT_EQ(stringArray.get(2), "World");
    EXPECT_EQ(stringArray.get(3, "Default"), "Default");
}
