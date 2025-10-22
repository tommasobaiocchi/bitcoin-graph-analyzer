# Bitcoin Graph Analyzer

C++ project for parsing and analyzing Bitcoin transaction data through directed and undirected graph representations. 

## Project Overview

This tool reads Bitcoin transaction data from `.csv` files and constructs graphs that model the relationships between transactions and addresses. The program performs the following tasks:

- Efficiently parses the transaction, input, and output files.
- Builds:
  - A **directed graph**, representing transaction flows.
  - An **undirected graph**, representing address connectivity.
- Computes:
  - The **largest connected component** of the undirected graph. 
  - Whether the **directed graph is acyclic**.
  - Whether **there exists a path** between two given addresses with a total flow exceeding a given threshold.

The project prioritizes **performance**, **low memory usage**, and **clean architecture**.

---

## File Structure

- `main.cpp`: The main source file containing all parsing and analysis logic.
- `inputs_small.csv`: Sample input file representing the inputs of transactions.
- `outputs_small.csv`: Sample output file representing the outputs of transactions.
- `transactions_small.csv`: Sample file containing transaction metadata (e.g., transaction IDs and timestamps).

---

## How It Works

1. **Parsing**:
   - The program uses fast C++ I/O (`ifstream`, `getline`, or `scanf`) to parse CSV files.
   - File names must be exactly: `transactions_small.csv`, `inputs_small.csv`, `outputs_small.csv`.

2. **Graph Construction**:
   - Addresses are mapped to integer IDs for efficient indexing.
   - The directed graph maps transaction flows: address → transaction → address.

3. **Graph Algorithms**:
   - **DFS** is used to compute the largest connected component.
   - **Cycle detection** on the directed graph is performed via DFS.
   - **Path search with threshold** uses DFS with accumulated flow tracking.

---


   ```bash
   g++ -O2 -std=c++17 -o bitcoin_analyzer main.cpp
