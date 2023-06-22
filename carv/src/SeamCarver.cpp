#include "SeamCarver.h"

#include <cmath>
#include <iostream>

SeamCarver::SeamCarver(Image image)
    : m_image(std::move(image))
{
}

const Image & SeamCarver::GetImage() const
{
    return m_image;
}

size_t SeamCarver::GetImageWidth() const
{
    return m_image.m_table.size();
}

size_t SeamCarver::GetImageHeight() const
{
    return m_image.m_table[0].size();
}

double SeamCarver::GetPixelEnergy(size_t x, size_t y) const
{
    int Rx, Gx, Bx;
    int Ry, Gy, By;
    Rx = m_image.m_table[(x - 1 + GetImageWidth()) % GetImageWidth()][y].m_red - m_image.m_table[(x + 1) % GetImageWidth()][y].m_red;
    Gx = m_image.m_table[(x - 1 + GetImageWidth()) % GetImageWidth()][y].m_green - m_image.m_table[(x + 1) % GetImageWidth()][y].m_green;
    Bx = m_image.m_table[(x - 1 + GetImageWidth()) % GetImageWidth()][y].m_blue - m_image.m_table[(x + 1) % GetImageWidth()][y].m_blue;
    Ry = m_image.m_table[x][(y - 1 + GetImageHeight()) % GetImageHeight()].m_red - m_image.m_table[x][(y + 1) % GetImageHeight()].m_red;
    Gy = m_image.m_table[x][(y - 1 + GetImageHeight()) % GetImageHeight()].m_green - m_image.m_table[x][(y + 1) % GetImageHeight()].m_green;
    By = m_image.m_table[x][(y - 1 + GetImageHeight()) % GetImageHeight()].m_blue - m_image.m_table[x][(y + 1) % GetImageHeight()].m_blue;
    return sqrt(Rx * Rx + Gx * Gx + Bx * Bx + Ry * Ry + Gy * Gy + By * By);
}

SeamCarver::Seam SeamCarver::FindHorizontalSeam() const
{
    std::vector<std::vector<Image::Pixel>> tabletka;
    for (size_t i = 0; i < m_image.m_table[0].size(); i++) {
        tabletka.push_back({});
        for (size_t j = 0; j < m_image.m_table.size(); j++) {
            tabletka[i].push_back(m_image.m_table[j][i]);
        }
    }
    Image rotatedImage = Image(tabletka);
    SeamCarver rot(rotatedImage);
    return rot.FindVerticalSeam();
}

SeamCarver::Seam SeamCarver::FindVerticalSeam() const
{
    SeamCarver::Seam seam(GetImageHeight());
    std::vector<std::vector<double>> distTo;
    std::vector<std::vector<short>> edgeTo;
    distTo.resize(GetImageHeight());
    edgeTo.resize(GetImageHeight());
    for (size_t i = 0; i < distTo.size(); i++) {
        distTo[i].resize(GetImageWidth());
        edgeTo[i].resize(GetImageWidth());
    }

    for (size_t i = 0; i < GetImageHeight(); ++i) {
        for (size_t j = 0; j < GetImageWidth(); ++j) {
            if (i == 0)
                distTo[i][j] = GetPixelEnergy(j, i);
            else
                distTo[i][j] = 21470000000000;
            edgeTo[i][j] = 0;
        }
    }

    for (size_t row = 0; row < GetImageHeight() - 1; ++row) {
        for (size_t col = 0; col < GetImageWidth(); ++col) {

            if (col != 0) {
                if (distTo[row + 1][col - 1] > distTo[row][col] + GetPixelEnergy(col - 1, row + 1)) {
                    distTo[row + 1][col - 1] = distTo[row][col] + GetPixelEnergy(col - 1, row + 1);
                    edgeTo[row + 1][col - 1] = 1;
                }
            }

            if (distTo[row + 1][col] > distTo[row][col] + GetPixelEnergy(col, row + 1)) {
                distTo[row + 1][col] = distTo[row][col] + GetPixelEnergy(col, row + 1);
                edgeTo[row + 1][col] = 0;
            }

            if (col != GetImageWidth() - 1) {
                if (distTo[row + 1][col + 1] > distTo[row][col] + GetPixelEnergy(col + 1, row + 1)) {
                    distTo[row + 1][col + 1] = distTo[row][col] + GetPixelEnergy(col + 1, row + 1);
                    edgeTo[row + 1][col + 1] = -1;
                }
            }
        }
    }

    double min_index = 0, min = distTo[GetImageHeight() - 1][0];
    for (size_t i = 1; i < GetImageWidth(); ++i)
        if (distTo[GetImageHeight() - 1][i] < min) {
            min_index = i;
            min = distTo[GetImageHeight() - 1][i];
        }

    seam[GetImageHeight() - 1] = min_index;
    for (int i = GetImageHeight() - 1; i > 0; --i)
        seam[i - 1] = seam[i] + edgeTo[i][seam[i]];

    return seam;
}

void SeamCarver::RemoveHorizontalSeam(const Seam & s)
{
    for (size_t i = 0; i < m_image.m_table.size(); i++) {
        m_image.m_table[i].erase(m_image.m_table[i].begin() + s[i]);
    }
}

void SeamCarver::RemoveVerticalSeam(const Seam & s)
{
    std::vector<std::vector<Image::Pixel>> tabletka;
    for (size_t i = 0; i < m_image.m_table[0].size(); i++) {
        tabletka.push_back({});
        for (size_t j = 0; j < m_image.m_table.size(); j++) {
            tabletka[i].push_back(m_image.m_table[j][i]);
        }
    }

    for (size_t i = 0; i < s.size(); i++) {
        tabletka[i].erase(tabletka[i].begin() + s[i]);
    }
    for (size_t i = 0; i < m_image.m_table.size(); i++) {
        for (size_t j = 0; j < m_image.m_table[0].size() - 1; j++) {
            m_image.m_table[i][j] = tabletka[j][i];
        }
    }
    m_image.m_table.resize(m_image.m_table.size() - 1);
}
