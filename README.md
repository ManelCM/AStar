# A* Algorithm Implementation

This repository contains an implementation of the A* algorithm to find the shortest path between two nodes in a graph defined by the 'Nodes.csv' and 'Carrers.csv' files (as seen in Practice 9). The resulting path is represented as a list of nodes connected according to the information in the 'Carrers.csv' file, and the length of the path is the sum of the distances between these nodes, assuming a straight line connection.

## Features

The implementation of the A* algorithm includes the following features:

- Reading 'Nodes.csv' and 'Carrers.csv' files.
- Calculating distances between two nodes, whether adjacent or not.
- Adding and removing elements from linked lists .
- Implementation of a priority queue (open list) for tracking visited nodes that haven't been expanded yet.

## Usage

You can run the program from the command line with the following arguments:

bash
python astar.py <start_node> <end_node>


- `start_node`: The starting node for the path.
- `end_node`: The destination node for the path.

The program will display information for each node it encounters along the path, including node identification, latitude, longitude, and the distance traveled.
