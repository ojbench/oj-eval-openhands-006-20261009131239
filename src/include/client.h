#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
// Global variables to store game state
char** game_map;        // Current state of the map ('?' = unknown, '0'-'8' = visited with count, 'X' = marked non-mine, '@' = marked mine)
bool** is_solved;       // Tracks which cells have been determined to be safe or mines
bool** is_safe;         // Tracks which cells are determined to be safe
bool** is_mine;         // Tracks which cells are determined to be mines

void InitGame() {
  // Initialize all global variables
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  
  // Allocate memory for game state
  game_map = new char*[rows];
  is_solved = new bool*[rows];
  is_safe = new bool*[rows];
  is_mine = new bool*[rows];
  
  for (int i = 0; i < rows; i++) {
    game_map[i] = new char[columns];
    is_solved[i] = new bool[columns];
    is_safe[i] = new bool[columns];
    is_mine[i] = new bool[columns];
    
    for (int j = 0; j < columns; j++) {
      game_map[i][j] = '?';
      is_solved[i][j] = false;
      is_safe[i][j] = false;
      is_mine[i][j] = false;
    }
  }
  
  // Execute the first move
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  // Read the current state of the map from stdin
  for (int i = 0; i < rows; i++) {
    std::string line;
    std::cin >> line;
    for (int j = 0; j < columns; j++) {
      game_map[i][j] = line[j];
    }
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  // First, update solved status based on current map
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j] != '?' && game_map[i][j] != 'X' && game_map[i][j] != '@') {
        // This cell is visited and shows a number
        int num = game_map[i][j] - '0';
        
        // Count unknown and marked neighbors
        int unknown_count = 0;
        int marked_count = 0;
        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            
            int ni = i + di;
            int nj = j + dj;
            
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns) {
              if (game_map[ni][nj] == '?') {
                unknown_count++;
              } else if (game_map[ni][nj] == '@') {
                marked_count++;
              }
            }
          }
        }
        
        // If all mines are marked, this cell is solved
        if (marked_count == num) {
          is_solved[i][j] = true;
        }
      }
    }
  }
  
  // Check if game is already over (all non-mine cells visited)
  bool all_non_mine_visited = true;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j] == '?') {
        all_non_mine_visited = false;
        break;
      }
    }
    if (!all_non_mine_visited) break;
  }
  
  if (all_non_mine_visited) {
    // Game should be over, but if not, we can't make any moves
    return;
  }
  
  // Look for obvious moves (Baseline1 strategy)
  // 1. Mark mines: if a numbered cell has exactly as many unknown neighbors as its number
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j] != '?' && game_map[i][j] != 'X' && game_map[i][j] != '@') {
        int num = game_map[i][j] - '0';
        
        // Count unknown neighbors
        int unknown_count = 0;
        std::vector<std::pair<int, int>> unknown_neighbors;
        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            
            int ni = i + di;
            int nj = j + dj;
            
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && game_map[ni][nj] == '?') {
              unknown_count++;
              unknown_neighbors.push_back({ni, nj});
            }
          }
        }
        
        // If number equals unknown count, mark all as mines
        if (num == unknown_count && unknown_count > 0) {
          // Mark the first unknown neighbor as a mine
          Execute(unknown_neighbors[0].first, unknown_neighbors[0].second, 1);
          return;
        }
      }
    }
  }
  
  // 2. Auto-explore: if a numbered cell has all mines marked
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j] != '?' && game_map[i][j] != 'X' && game_map[i][j] != '@') {
        int num = game_map[i][j] - '0';
        
        // Count marked neighbors
        int marked_count = 0;
        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            
            int ni = i + di;
            int nj = j + dj;
            
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && game_map[ni][nj] == '@') {
              marked_count++;
            }
          }
        }
        
        // If all mines are marked, auto-explore
        if (marked_count == num && num > 0) {
          Execute(i, j, 2);
          return;
        }
      }
    }
  }
  
  // 3. Visit safe cells: if we've determined a cell is safe
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j] == '?' && is_safe[i][j]) {
        Execute(i, j, 0);
        return;
      }
    }
  }
  
  // 4. Advanced deduction: look for subset relationships
  // For each numbered cell, check if its unknown neighbors are a subset of another's
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j] != '?' && game_map[i][j] != 'X' && game_map[i][j] != '@') {
        int num = game_map[i][j] - '0';
        
        // Get unknown neighbors of current cell
        std::vector<std::pair<int, int>> current_unknown;
        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            
            int ni = i + di;
            int nj = j + dj;
            
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && game_map[ni][nj] == '?') {
              current_unknown.push_back({ni, nj});
            }
          }
        }
        
        // Compare with other numbered cells
        for (int i2 = 0; i2 < rows; i2++) {
          for (int j2 = 0; j2 < columns; j2++) {
            if ((i2 != i || j2 != j) && game_map[i2][j2] != '?' && game_map[i2][j2] != 'X' && game_map[i2][j2] != '@') {
              int num2 = game_map[i2][j2] - '0';
              
              // Get unknown neighbors of other cell
              std::vector<std::pair<int, int>> other_unknown;
              for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                  if (di == 0 && dj == 0) continue;
                  
                  int ni = i2 + di;
                  int nj = j2 + dj;
                  
                  if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && game_map[ni][nj] == '?') {
                    other_unknown.push_back({ni, nj});
                  }
                }
              }
              
              // Check if current_unknown is a subset of other_unknown
              if (current_unknown.size() < other_unknown.size()) {
                bool is_subset = true;
                for (auto& cell : current_unknown) {
                  bool found = false;
                  for (auto& other_cell : other_unknown) {
                    if (cell.first == other_cell.first && cell.second == other_cell.second) {
                      found = true;
                      break;
                    }
                  }
                  if (!found) {
                    is_subset = false;
                    break;
                  }
                }
                
                if (is_subset) {
                  // Calculate difference in mine counts
                  int mine_diff = num2 - num;
                  
                  // The difference must be non-negative
                  if (mine_diff >= 0) {
                    // The cells in other_unknown but not in current_unknown must contain exactly mine_diff mines
                    std::vector<std::pair<int, int>> diff_cells;
                    for (auto& cell : other_unknown) {
                      bool in_current = false;
                      for (auto& current_cell : current_unknown) {
                        if (cell.first == current_cell.first && cell.second == current_cell.second) {
                          in_current = true;
                          break;
                        }
                      }
                      if (!in_current) {
                        diff_cells.push_back(cell);
                      }
                    }
                    
                    // If mine_diff equals the number of diff_cells, they are all mines
                    if (mine_diff == diff_cells.size() && mine_diff > 0) {
                      Execute(diff_cells[0].first, diff_cells[0].second, 1);
                      return;
                    }
                    
                    // If mine_diff is 0, they are all safe
                    if (mine_diff == 0 && diff_cells.size() > 0) {
                      // Mark all as safe
                      for (auto& cell : diff_cells) {
                        is_safe[cell.first][cell.second] = true;
                      }
                      Execute(diff_cells[0].first, diff_cells[0].second, 0);
                      return;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  
  // 4. Advanced deduction: look for subset relationships
  // For each numbered cell, check if its unknown neighbors are a subset of another's
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j] != '?' && game_map[i][j] != 'X' && game_map[i][j] != '@') {
        int num = game_map[i][j] - '0';
        
        // Get unknown neighbors of current cell
        std::vector<std::pair<int, int>> current_unknown;
        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            
            int ni = i + di;
            int nj = j + dj;
            
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && game_map[ni][nj] == '?') {
              current_unknown.push_back({ni, nj});
            }
          }
        }
        
        // Compare with other numbered cells
        for (int i2 = 0; i2 < rows; i2++) {
          for (int j2 = 0; j2 < columns; j2++) {
            if ((i2 != i || j2 != j) && game_map[i2][j2] != '?' && game_map[i2][j2] != 'X' && game_map[i2][j2] != '@') {
              int num2 = game_map[i2][j2] - '0';
              
              // Get unknown neighbors of other cell
              std::vector<std::pair<int, int>> other_unknown;
              for (int di = -1; di <= 1; di++) {
                for (int dj = -1; dj <= 1; dj++) {
                  if (di == 0 && dj == 0) continue;
                  
                  int ni = i2 + di;
                  int nj = j2 + dj;
                  
                  if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && game_map[ni][nj] == '?') {
                    other_unknown.push_back({ni, nj});
                  }
                }
              }
              
              // Check if current_unknown is a subset of other_unknown
              if (current_unknown.size() < other_unknown.size()) {
                bool is_subset = true;
                for (auto& cell : current_unknown) {
                  bool found = false;
                  for (auto& other_cell : other_unknown) {
                    if (cell.first == other_cell.first && cell.second == other_cell.second) {
                      found = true;
                      break;
                    }
                  }
                  if (!found) {
                    is_subset = false;
                    break;
                  }
                }
                
                if (is_subset) {
                  // Calculate difference in mine counts
                  int mine_diff = num2 - num;
                  
                  // The difference must be non-negative
                  if (mine_diff >= 0) {
                    // The cells in other_unknown but not in current_unknown must contain exactly mine_diff mines
                    std::vector<std::pair<int, int>> diff_cells;
                    for (auto& cell : other_unknown) {
                      bool in_current = false;
                      for (auto& current_cell : current_unknown) {
                        if (cell.first == current_cell.first && cell.second == current_cell.second) {
                          in_current = true;
                          break;
                        }
                      }
                      if (!in_current) {
                        diff_cells.push_back(cell);
                      }
                    }
                    
                    // If mine_diff equals the number of diff_cells, they are all mines
                    if (mine_diff == diff_cells.size() && mine_diff > 0) {
                      Execute(diff_cells[0].first, diff_cells[0].second, 1);
                      return;
                    }
                    
                    // If mine_diff is 0, they are all safe
                    if (mine_diff == 0 && diff_cells.size() > 0) {
                      Execute(diff_cells[0].first, diff_cells[0].second, 0);
                      return;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  
  // 5. If no logical move found, make an educated guess
  // Find the cell with the lowest probability of being a mine
  double min_prob = 1.0;
  int best_r = -1, best_c = -1;
  
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (game_map[i][j] == '?') {
        // Calculate probability based on neighboring numbered cells
        double prob = 0.0;
        int constraint_count = 0;
        
        for (int di = -1; di <= 1; di++) {
          for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) continue;
            
            int ni = i + di;
            int nj = j + dj;
            
            if (ni >= 0 && ni < rows && nj >= 0 && nj < columns && 
                game_map[ni][nj] != '?' && game_map[ni][nj] != 'X' && game_map[ni][nj] != '@') {
              
              int num = game_map[ni][nj] - '0';
              // Count unknown and marked neighbors of the numbered cell
              int unknown_count = 0;
              int marked_count = 0;
              for (int di2 = -1; di2 <= 1; di2++) {
                for (int dj2 = -1; dj2 <= 1; dj2++) {
                  if (di2 == 0 && dj2 == 0) continue;
                  
                  int ni2 = ni + di2;
                  int nj2 = nj + dj2;
                  
                  if (ni2 >= 0 && ni2 < rows && nj2 >= 0 && nj2 < columns) {
                    if (game_map[ni2][nj2] == '?') {
                      unknown_count++;
                    } else if (game_map[ni2][nj2] == '@') {
                      marked_count++;
                    }
                  }
                }
              }
              
              // Probability contribution from this constraint
              if (unknown_count > 0) {
                double cell_prob = (num - marked_count) / (double)unknown_count;
                prob += cell_prob;
                constraint_count++;
              }
            }
          }
        }
        
        // Average probability
        if (constraint_count > 0) {
          prob /= constraint_count;
        } else {
          // If no constraints, use overall mine density
          prob = (total_mines / 2.0) / (rows * columns);  // Conservative estimate
        }
        
        if (prob < min_prob) {
          min_prob = prob;
          best_r = i;
          best_c = j;
        }
      }
    }
  }
  
  // Make the guess
  if (best_r != -1) {
    Execute(best_r, best_c, 0);
  } else {
    // Fallback: just visit the first unknown cell
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < columns; j++) {
        if (game_map[i][j] == '?') {
          Execute(i, j, 0);
          return;
        }
      }
    }
  }
}

#endif