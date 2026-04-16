#ifndef NFA_H
#define NFA_H

#include "regex.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_STATES 64

/**
 * @brief Struct to represent an alphabet. It contains an array of symbols and a mapping from characters
 * to their corresponding column index in the symbols array. The symbol_count field keeps track of how many
 * unique symbols are in the alphabet.
 */
struct alphabet
{
    /* Array of symbols in the alphabet. The index of each
    symbol corresponds to its column in the transition table. */
    char symbols[256];
    /* Mapping from character to column index in the symbols array */
    int char_to_col[256];
    /* Number of symbols in the alphabet */
    int symbol_count;
};
typedef struct alphabet alphabet;

/**
 * @brief Struct to represent a non-deterministic finite automaton (NFA). It contains the start state,
 * a bitset representing the accept states, the total number of states, the alphabet used by the NFA,
 * a transition table, and a cache for epsilon closures. The transition table is a 2D array where each
 * entry is a bitset representing the set of states reachable from the current state on the given symbol.
 */
struct NFA
{
    /* State id for the start state */
    uint8_t start_state;
    /* Bitset representing accept states */
    uint64_t accept_states;
    /* Number of states in the NFA */
    uint8_t states;
    /* Alphabet used by the NFA */
    alphabet nfa_alphabet;
    /** Transition table. Each entry is a bitset representing the set of
     * states reachable from the current state on the given symbol. A
     * MAX_STATES x nfa_alphabet.symbol_count matrix. */
    uint64_t** transitions;
    /* Cache for epsilon closures. Each entry is a bitset representing
    the epsilon closure of the corresponding state. */
    uint64_t* epsilon_closure_cache;
};
typedef struct NFA nfa;

/**
 * @brief Convert a regular expression represented as a regex struct into an NFA.
 * This function uses a stack-based approach to construct the NFA from the postfix
 * representation of the regex.
 * @param r The input regular expression as a regex struct
 * @return An NFA struct representing the non-deterministic finite automaton
 * for the given regex.
 */
nfa regex_to_nfa(const regex r);

/**
 * @brief Function to check if a given input string matches the language defined by the NFA.
 * This function simulates the NFA on the input string and returns true if the NFA accepts
 * the string, and false otherwise.
 * @param automaton The NFA to simulate
 * @param input The input string to check against the NFA
 * @param input_length The length of the input string
 * @return true if the NFA accepts the input string, false otherwise
 */
bool match_nfa(nfa automaton, const char *input, size_t input_length);

/**
 * @brief Serialize an NFA to a binary file.
 * The serialized format stores metadata, alphabet symbols and transition table.
 * @param automaton Pointer to the NFA to serialize
 * @param file_path Output file path
 * @return true if the file was written successfully, false otherwise
 */
bool save_nfa(const nfa *automaton, const char *file_path);

/**
 * @brief Release heap memory owned by an NFA.
 * @param automaton Pointer to the NFA to free
 */
void free_nfa(nfa *automaton);

#endif // NFA_H