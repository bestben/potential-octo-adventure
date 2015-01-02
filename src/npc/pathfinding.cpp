#include "pathfinding.hpp"
#include "../chunkmanager.h"

#include <iostream>
#include <cmath>
#include <stdexcept>

using Cell = Pathfinding::Cell;

Pathfinding::Pathfinding(ChunkManager& grid) : mGrid{grid}, mLastDir{} {

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

    std::vector<Cell> path;
    /* si la destination est atteinte, on remonte le chemin */
    if (current == end) {
        path = getPathFromClosedMap();
    } else{
        path.clear();
        #ifdef QT_DEBUG
        std::cout << "Impossible de trouver un chemin dans la grille" << std::endl;
        #endif
    }


    return path;
}

std::vector<Cell> Pathfinding::getPathFromClosedMap() {
    std::vector<Cell> path;
    /* l'arrivée est le dernier élément de la liste fermée */
    Node& tmp = mClosedNodes[mEnd];

    path.push_back(mEnd);

    while (tmp.parent != mStart) {
        path.push_back(tmp.parent);
        tmp = mClosedNodes[tmp.parent];
    }

    //path.push_back(mStart);

    /*for (unsigned int i = 0; i < path.size() / 2; ++i) {
        Cell tmpCell = path[i];
        path[i] = path[path.size() - i - 1];
        path[path.size() - i - 1] = tmpCell;
    }*/

    return path;
}

Cell Pathfinding::getBestNode() {
    long m_coutf = mOpenedNodes.begin()->second.cout_f;
    Cell cell = mOpenedNodes.begin()->first;

    for (auto& it : mOpenedNodes)
        if (it.second.cout_f < m_coutf){
            m_coutf = it.second.cout_f;
            cell = it.first;
        }

    return cell;
}

long Pathfinding::distance(Cell c1, Cell c2) {
    return std::abs(c1.i - c2.i) +
            std::abs(c1.j - c2.j) +
            std::abs(c1.k - c2.k);
}

bool Pathfinding::isInside(std::map<Cell, Node>& nodeMap, Cell cell) {
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
    std::vector<Cell> dir{
        {0,  0, -1}
    };
    bool loaded = true;
    VoxelType type = mGrid.getVoxel(cell.i, cell.j - 1, cell.k, &loaded).type;
    if (!loaded) {
        std::cout << "not loaded" << std::endl;
        return true;
    }
    //if (type != VoxelType::AIR) {
        dir.push_back({0,  1,  0});
        dir.push_back({0,  -1,  0});
        dir.push_back({1,  0,  0});
        dir.push_back({-1,  0,  0});
    //}

    int maxUpCell = 2;
    std::vector<Cell> cells = getLastNodes(cell, maxUpCell);
    int n = 1;
    for (unsigned int i = 0; i < (cells.size() - 1); ++i) {
        int dz = cells[i].k - cells[i + 1].k;
        if (dz != 0) {
            n++;
        }
    }
    if (n < maxUpCell) {
        dir.push_back({0,  0,  1});
    }

    for (Coords& d : dir) {
        Cell pos = cell + d;
        if (!isInside(mClosedNodes, pos)) {
            VoxelType type = mGrid.getVoxel(pos.i, pos.j, pos.k, &loaded).type;
            if (!loaded) {
                std::cout << "not loaded" << std::endl;
                return true;
            }
            if ((type == VoxelType::AIR) || ((type != VoxelType::AIR) && (pos == mEnd))) {
                Node n;
                n.cout_g = mClosedNodes[cell].cout_g + distance(cell, pos);
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
