#include <cairomm/cairomm.h>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <iostream>

// 手动求解线性方程组 Ax = b，使用高斯消元法
std::vector<double> solveLinearSystem(const std::vector<std::vector<double>>& A, const std::vector<double>& b) {
    int n = A.size();
    if (n == 0 || A[0].size() != n || b.size() != n) {
        throw std::invalid_argument("矩阵 A 和向量 b 的维度不匹配");
    }

    // 构造增广矩阵 [A | b]
    std::vector<std::vector<double>> augmentedMatrix(n, std::vector<double>(n + 1));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            augmentedMatrix[i][j] = A[i][j];
        }
        augmentedMatrix[i][n] = b[i];
    }

    // 高斯消元法
    for (int i = 0; i < n; ++i) {
        // 主元素选择（避免除以零）
        int maxRow = i;
        for (int k = i + 1; k < n; ++k) {
            if (std::abs(augmentedMatrix[k][i]) > std::abs(augmentedMatrix[maxRow][i])) {
                maxRow = k;
            }
        }
        if (augmentedMatrix[maxRow][i] == 0) {
            throw std::runtime_error("矩阵奇异，无法求解方程组");
        }

        // 交换当前行和最大行
        std::swap(augmentedMatrix[i], augmentedMatrix[maxRow]);

        // 消元过程
        for (int j = i + 1; j < n; ++j) {
            double factor = augmentedMatrix[j][i] / augmentedMatrix[i][i];
            for (int k = i; k <= n; ++k) {
                augmentedMatrix[j][k] -= factor * augmentedMatrix[i][k];
            }
        }
    }

    // 回代求解
    std::vector<double> x(n, 0);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = augmentedMatrix[i][n] / augmentedMatrix[i][i];
        for (int j = i - 1; j >= 0; --j) {
            augmentedMatrix[j][n] -= augmentedMatrix[j][i] * x[i];
        }
    }

    return x;
}

// 透视变换矩阵计算函数
Cairo::Matrix computePerspectiveMatrix(
    const std::vector<std::pair<double, double>>& touchPoints,
    const std::vector<std::pair<double, double>>& screenPoints) {
    // 确保点的数量正确
    if (touchPoints.size() != 4 || screenPoints.size() != 4) {
        throw std::invalid_argument("需要四个点");
    }

    // 构建线性方程组的矩阵 A 和向量 b
    std::vector<std::vector<double>> A(8, std::vector<double>(8, 0));
    std::vector<double> b(8);

    for (int i = 0; i < 4; ++i) {
        double x = touchPoints[i].first;
        double y = touchPoints[i].second;
        double x_prime = screenPoints[i].first;
        double y_prime = screenPoints[i].second;

        // A 矩阵的前四行
        A[i * 2][0] = x;
        A[i * 2][1] = y;
        A[i * 2][2] = 1;
        A[i * 2][3] = 0;
        A[i * 2][4] = 0;
        A[i * 2][5] = 0;
        A[i * 2][6] = -x * x_prime;
        A[i * 2][7] = -y * x_prime;
        b[i * 2] = x_prime;

        // A 矩阵的后四行
        A[i * 2 + 1][0] = 0;
        A[i * 2 + 1][1] = 0;
        A[i * 2 + 1][2] = 0;
        A[i * 2 + 1][3] = x;
        A[i * 2 + 1][4] = y;
        A[i * 2 + 1][5] = 1;
        A[i * 2 + 1][6] = -x * y_prime;
        A[i * 2 + 1][7] = -y * y_prime;
        b[i * 2 + 1] = y_prime;
    }

    // 求解线性方程组
    std::vector<double> solution = solveLinearSystem(A, b);

    // 设置透视变换矩阵 H
    Cairo::Matrix H;
    H.xx = solution[0];
    H.yx = solution[1];
    H.xy = solution[3];
    H.yy = solution[4];
    H.x0 = solution[2];
    H.y0 = solution[5];

    return H;
}

int main() {
    // 输入触摸屏的四个点
    std::vector<std::pair<double, double>> touchPoints = {
        {0, 0}, {320, 0}, {320, 240}, {0, 240}
    };

    // 输入屏幕的四个点
    std::vector<std::pair<double, double>> screenPoints = {
        {0, 0}, {640, 0}, {640, 480}, {0, 480}
    };

    // 计算透视变换矩阵
    Cairo::Matrix H = computePerspectiveMatrix(touchPoints, screenPoints);

    // 输出结果
    std::cout << "透视变换矩阵 (H):" << std::endl;
    std::cout << H.xx<<","<<H.yx<<std::endl
     <<H.xy<<","<<H.yy<<std::endl
     <<H.x0<<","<<H.y0<<std::endl;

    return 0;
}
