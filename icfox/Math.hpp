#pragma once
#include <windows.h>
#include <cmath>
#include <algorithm>

struct Vector2 {
    float x, y;

    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}

    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(float s) const { return Vector2(x * s, y * s); }
    Vector2 operator/(float s) const { return Vector2(x / s, y / s); }
};

struct Vector3 {
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    Vector3 operator/(float s) const { return Vector3(x / s, y / s, z / s); }

    float Dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    float Distance(const Vector3& other) const {
        Vector3 diff = *this - other;
        return sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
    }

    float Length() const {
        return sqrtf(x * x + y * y + z * z);
    }
};

struct Matrix4x4 {
    float m[4][4];

    Matrix4x4() {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m[i][j] = 0.0f;
    }

    Vector3 Multiply(const Vector3& v) const {
        Vector3 result;
        result.x = v.x * m[0][0] + v.y * m[1][0] + v.z * m[2][0] + m[3][0];
        result.y = v.x * m[0][1] + v.y * m[1][1] + v.z * m[2][1] + m[3][1];
        result.z = v.x * m[0][2] + v.y * m[1][2] + v.z * m[2][2] + m[3][2];
        float w = v.x * m[0][3] + v.y * m[1][3] + v.z * m[2][3] + m[3][3];

        if (w < 0.001f) return Vector3(0, 0, 0);

        float invW = 1.0f / w;
        result.x *= invW;
        result.y *= invW;
        result.z *= invW;

        return result;
    }
};

namespace Math {
    // World to screen using view matrix
    bool WorldToScreen(const Vector3& worldPos, Vector2& screenPos, const Matrix4x4& viewMatrix, int screenWidth, int screenHeight) {
        Vector3 transformed = viewMatrix.Multiply(worldPos);

        if (transformed.z < 0.001f)
            return false;

        screenPos.x = (screenWidth / 2.0f) + (transformed.x * (screenWidth / 2.0f));
        screenPos.y = (screenHeight / 2.0f) - (transformed.y * (screenHeight / 2.0f));

        return true;
    }

    // Get bounding box corners for a player (approximate from head and root position)
    // Returns top and bottom screen positions
    bool GetBoundingBox(const Vector3& headPos, const Vector3& rootPos, Vector2& top, Vector2& bottom,
        const Matrix4x4& viewMatrix, int screenWidth, int screenHeight) {

        Vector2 headScreen, rootScreen;
        if (!WorldToScreen(headPos, headScreen, viewMatrix, screenWidth, screenHeight))
            return false;
        if (!WorldToScreen(rootPos, rootScreen, viewMatrix, screenWidth, screenHeight))
            return false;

        top = headScreen;
        bottom = rootScreen;

        return true;
    }

    // Calculate box dimensions from head and root screen positions
    void GetBoxDimensions(const Vector2& head, const Vector2& root, float& boxWidth, float& boxHeight) {
        boxHeight = abs(head.y - root.y);
        boxWidth = boxHeight * 0.5f; // Approximate width based on height
        if (boxWidth < 10.0f) boxWidth = 10.0f;
    }
}