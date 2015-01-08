#include "pathfinding.hpp"
#include "../chunkmanager.h"

#include <iostream>
#include <cmath>
#include <stdexcept>

using Cell = Pathfinding::Cell;

Pathfinding::Pathfinding(ChunkManager& grid, int maxLength) : mGrid{grid}, mLastDir{}, m_maxLength{maxLength}{
    m_path.reserve(m_maxLength);
}

std::vector<Cell> Pathfinding::getPath(Cell start, Cell end) {
    mOpenedNodes.clear();
    mClosedNodes.clear();

    mStart = start;
    mEnd = end;

    Node startNode{0, 0, 0, start};
    startNode.parent = start;
    Cell current = start;

    mOpenedNodes[current] = startNode;
    addToClosedMap(current);
    addNeighbors(current);

    bool error = false;

    while((current != mEnd) && !mOpenedNodes.empty() && !error) {
        /* on cherche le meilleur noeud de la liste ouverte, on sait qu'elle n'est pas vide donc il existe */
        current = getBestNode();

        /* on le passe dans la liste fermee, il ne peut pas déjà y être */
        addToClosedMap(current);

        /* on recommence la recherche des noeuds adjacents */
        error = addNeighbors(current);
    }

    /* si la destination est atteinte, on remonte le chemin */
    if (current == end) {
        getPathFromClosedMap();
    } else {
        m_path.clear();
        #ifdef QT_DEBUG
        std::cout << "Impossible de trouver un chemin dans la grille" << std::endl;
        #endif
    }


    return m_path;
}

void Pathfinding::getPathFromClosedMap() {
    m_path.clear();
    if (m_path.capacity() < m_maxLength) {
        m_path.reserve(m_maxLength);
    }
    /* l'arrivée est le dernier élément de la liste fermée */
    Node& tmp = mClosedNodes[mEnd];

    m_path.push_back(mEnd);

    while (tmp.parent != mStart) {
        m_path.push_back(tmp.parent);
        tmp = mClosedNodes[tmp.parent];
    }
}

Cell Pathfinding::getBestNode() {
    long m_coutf = mOpenedNodes.begin()->second.cout_f;
    Cell cell = mOpenedNodes.begin()->first;

    for (auto& it : mOpenedNodes) {
        if (it.second.cout_f < m_coutf){
            m_coutf = it.second.cout_f;
            cell = it.first;
        }
    }

    return cell;
}

long Pathfinding::distance(Cell c1, Cell c2) {
    return std::abs(c1.i - c2.i) +
            std::abs(c1.j - c2.j) +
            std::abs(c1.k - c2.k);
}

bool Pathfinding::isInside(std::unordered_map<Cell, Node>& nodeMap, Cell cell) {
    return nodeMap.find(cell) != nodeMap.end();
}

std::vector<Cell> Pathfinding::getLastNodes(Cell cell, int n) {
    std::vector<Cell> nodes;
    Node tmp = mClosedNodes.at(cell);
    nodes.push_back(cell);
    int i = 1;
    while ((tmp.parent != mStart) && (i < n)) {
        nodes.push_back(tmp.parent);
        tmp = mClosedNodes.at(tmp.parent);
        i++;
    }
    return nodes;
}

bool Pathfinding::addNeighbors(Cell cell) {
    static std::vector<Cell> dir{
        {0,  -1, 0},
        {0,  0,  1},
        {0,  0,  -1},
        {1,  0,  0},
        {-1,  0,  0},
        {0,  1,  0}
    };
    int dirCount = 5;
    bool loaded = true;
    VoxelType type = mGrid.getVoxel(cell.i, cell.j - 1, cell.k, &loaded).type;
    if (!loaded) {
        return true;
    }
    if (type != VoxelType::AIR) {
        dirCount = 6;
    }

    for (int i = 0; i < dirCount; ++i) {
        Cell& d = dir[i];
        Cell pos = cell + d;
        if (!isInside(mClosedNodes, pos)) {
            VoxelType type = mGrid.getVoxel(pos.i, pos.j, pos.k, &loaded).type;
            if (!loaded) {
                return true;
            }
            VoxelType bottomType = mGrid.getVoxel(pos.i, pos.j - 1, pos.k, &loaded).type;
            if (!loaded) {
                return true;
            }
            bool isWalkable = (bottomType != VoxelType::WATER);
            if (((type == VoxelType::AIR) && isWalkable) ||
                ((type != VoxelType::AIR) && (pos == mEnd))) {
                Node n;
                n.cout_g = mClosedNodes[cell].cout_g + distance(cell, pos);
                if (n.cout_g > m_maxLength) {
                    break;
                }
                /* calcul du cout H du noeud à la destination */
                n.cout_h = distance(pos, mEnd);
                n.cout_f = n.cout_g + n.cout_h;
                n.parent = cell;

                if (isInside(mOpenedNodes, pos)) {
                    if (n.cout_f < mOpenedNodes[pos].cout_f) {
                        /* si le nouveau chemin est meilleur, on met à jour */
                        mOpenedNodes[pos] = n;
                    }
                } else {
                    mOpenedNodes[pos] = n;
                }
            }
        }
    }

    return false;
}

void Pathfinding::addToClosedMap(Cell cell) {
    mClosedNodes[cell] = mOpenedNodes[cell];

    if (mOpenedNodes.erase(cell) == 0) {
        std::cerr << "Erreur, le noeud n'apparait pas dans la liste ouverte, impossible à supprimer" << std::endl;
    }
}
