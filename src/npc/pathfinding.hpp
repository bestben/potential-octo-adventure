#pragma once

#include <vector>
#include <unordered_map>

#include "../chunk.h"

class ChunkManager;

/**
 * Permet de trouver un chemin dans une grille.
 */
class Pathfinding {
public:
    using Cell = Coords;

    Pathfinding(ChunkManager& grid, int maxLength);
    /**
     * Renvoie un chemin entre start et end.
     * NOTE : renvoie une exception si aucun chemin n'est trouvé.
     */
    std::vector<Cell> getPath(Cell start, Cell end);

private:
    struct Node {
        long cout_g; // Le cout pour aller du départ à this
        long cout_h; // Le cout pour aller de this à l'arrivée
        long cout_f; // La somme de cout_g et cout_h
        Cell parent; // Le parent de ce noeud
    };

    Cell getBestNode();

    /**
     * Renvoie la distance entre 2 case de la grille.
     * Note : On utilise la distance de Manhattan.
     */
    long distance(Cell c1, Cell c2);
    /**
     * Test si un noeud est déjà présent dans une map.
     */
    bool isInside(std::unordered_map<Cell, Node>& nodeMap, Cell cell);
    /**
     *
     */
    bool addNeighbors(Cell cell);
    /**
     * Transfère un noeud de la liste ouverte à la liste fermée.
     */
    void addToClosedMap(Cell cell);

    void getPathFromClosedMap();

    std::vector<Cell> getLastNodes(Cell cell, int n);

    // ATTRIBUTS

    ChunkManager& mGrid;
    std::unordered_map<Cell, Node> mOpenedNodes;
    std::unordered_map<Cell, Node> mClosedNodes;
    Cell mStart;
    Cell mEnd;

    Cell mLastDir;

    // La longueur maximum du chemin à rechercher
    int m_maxLength;

    std::vector<Cell> m_path;
};

