// This source can be used to check ASM with different compiler on godbolt.org
// It aims to expose ABI issues.

int mult(int num1, int num2, int num3, int num4, int num5, int num6, int num7, int num8) {
    return num1 * num2 * num3 * num4 * num5 * num6 * num7 * num8;
}

double add(int num1, float num2, int num3, double num4, int num5, int num6, float num7, int num8) {
    return num1 + num2 + num3 + num4 + num5 + num6 + num7 + num8;
}

double test()
{
    int m_res = mult(1, 2, 3, 4, 5, 6, 7, 8);
    double a_res = add(m_res, 2.0f, 3, 4.0, 5, 6, 7.0f, 8);
    return a_res;
}