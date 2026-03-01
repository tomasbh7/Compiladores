#include "nfa.h"
#include <stdlib.h>
#include <string.h>


/**
 * @brief Creates and initializes a new alphabet structure.
 *
 * Initializes the character-to-column mapping table with -1,
 * meaning no symbol is registered yet.
 *
 * The EPSILON symbol is always placed in column 0.
 *
 * @return A fully initialized alphabet structure.
 */
alphabet new_alphabet()
{
    alphabet a;

    for (int i = 0; i < 256; i++)
        a.char_to_col[i] = -1;

    a.symbol_count = 0;

    /* EPSILON en columna 0 */
    a.symbols[0] = EPSILON_SYMBOL;
    a.char_to_col[(unsigned char)EPSILON_SYMBOL] = 0;
    a.symbol_count = 1;

    return a;
}


/**
 * @brief Adds a symbol to the alphabet if it does not already exist.
 *
 * The function ignores:
 *  - The EPSILON symbol (already predefined at column 0)
 *  - Any symbol that is already present in the alphabet
 *
 * @param a Pointer to the alphabet structure.
 * @param symbol Character to add.
 */
void add_symbol(alphabet *a, char symbol)
{
    unsigned char s = (unsigned char)symbol;

    if (s == EPSILON_SYMBOL)
        return;

    if (a->char_to_col[s] != -1)
        return;

    uint16_t col = a->symbol_count;
    a->symbols[col] = symbol;
    a->char_to_col[s] = col;
    a->symbol_count++;
}


/**
 * @brief Initializes a new states manager.
 *
 * The states manager is responsible for:
 *  - Creating states
 *  - Storing transitions
 *  - Managing the alphabet
 *
 * @return A fully initialized states_manager.
 */
states_manager new_states_manager()
{
    states_manager manager;

    manager.next_id = 0;
    manager.states_count = 0;
    manager.transitions_count = 0;
    manager.manager_alphabet = new_alphabet();

    return manager;
}


/**
 * @brief Creates a new state and registers it in the manager.
 *
 * @param manager Pointer to the states manager.
 * @return The ID of the newly created state.
 */
uint8_t new_state(states_manager *manager)
{
    uint8_t id = manager->next_id++;

    manager->states[manager->states_count++] = id;

    return id;
}


/**
 * @brief Adds a transition to the states manager.
 *
 * The transition is stored internally and the symbol
 * is automatically registered in the alphabet.
 *
 * @param manager Pointer to the states manager.
 * @param from_state Origin state ID.
 * @param symbol Transition symbol.
 * @param to_state Destination state ID.
 */
void add_transition(states_manager *manager,
                    uint8_t from_state,
                    char symbol,
                    uint8_t to_state)
{
    t_transition t;

    t.from_state = from_state;
    t.symbol = symbol;
    t.to_state = to_state;

    manager->transitions[manager->transitions_count++] = t;

    add_symbol(&manager->manager_alphabet, symbol);
}


/**
 * @brief Creates an NFA fragment for a single symbol.
 *
 * Constructs:
 *     start --symbol--> end
 *
 * @param manager Pointer to the states manager.
 * @param symbol Input symbol.
 * @return NFA fragment representing the symbol.
 */
t_nfa symbol_nfa(states_manager *manager, char symbol)
{
    t_nfa result;

    result.start = new_state(manager);
    result.end   = new_state(manager);

    add_transition(manager, result.start, symbol, result.end);

    return result;
}


/**
 * @brief Concatenates two NFA fragments using Thompson's construction.
 *
 * Connects:
 *     a.end --ε--> b.start
 *
 * @param manager Pointer to the states manager.
 * @param a First NFA fragment.
 * @param b Second NFA fragment.
 * @return Resulting concatenated NFA fragment.
 */
t_nfa concat_nfa(states_manager *manager, t_nfa *a, t_nfa *b)
{
    add_transition(manager, a->end, EPSILON_SYMBOL, b->start);

    t_nfa result;
    result.start = a->start;
    result.end   = b->end;

    return result;
}


/**
 * @brief Creates the union (alternation) of two NFA fragments.
 * @param manager Pointer to the states manager.
 * @param a First NFA fragment.
 * @param b Second NFA fragment.
 * @return Resulting union NFA fragment.
 */
t_nfa union_nfa(states_manager *manager, t_nfa *a, t_nfa *b)
{
    t_nfa result;

    result.start = new_state(manager);
    result.end   = new_state(manager);

    add_transition(manager, result.start, EPSILON_SYMBOL, a->start);
    add_transition(manager, result.start, EPSILON_SYMBOL, b->start);

    add_transition(manager, a->end, EPSILON_SYMBOL, result.end);
    add_transition(manager, b->end, EPSILON_SYMBOL, result.end);

    return result;
}


/**
 * @brief Applies positive closure (+) to an NFA fragment.
 *
 * Structure:
 *     new_start --ε--> a.start
 *     a.end --ε--> a.start
 *     a.end --ε--> new_end
 *
 * Requires at least one occurrence.
 *
 * @param manager Pointer to the states manager.
 * @param a NFA fragment.
 * @return Resulting NFA fragment with positive closure.
 */
t_nfa positive_closure_nfa(states_manager *manager, t_nfa *a)
{
    t_nfa result;

    result.start = new_state(manager);
    result.end   = new_state(manager);

    add_transition(manager, result.start, EPSILON_SYMBOL, a->start);
    add_transition(manager, a->end, EPSILON_SYMBOL, a->start);
    add_transition(manager, a->end, EPSILON_SYMBOL, result.end);

    return result;
}


/**
 * @brief Applies Kleene closure (*) to an NFA fragment.
 *
 * Same as positive closure, but also allows zero occurrences:
 *     new_start --ε--> new_end
 *
 * @param manager Pointer to the states manager.
 * @param a NFA fragment.
 * @return Resulting NFA fragment with Kleene star.
 */
t_nfa kleene_closure_nfa(states_manager *manager, t_nfa *a)
{
    t_nfa result = positive_closure_nfa(manager, a);

    add_transition(manager, result.start, EPSILON_SYMBOL, result.end);

    return result;
}


/**
 * @brief Applies optional operator (?) to an NFA fragment.
 *
 * Allows zero or one occurrence.
 *
 * @param manager Pointer to the states manager.
 * @param a NFA fragment.
 * @return Resulting optional NFA fragment.
 */
t_nfa optional_nfa(states_manager *manager, t_nfa *a)
{
    t_nfa result;

    result.start = new_state(manager);
    result.end   = new_state(manager);

    add_transition(manager, result.start, EPSILON_SYMBOL, a->start);
    add_transition(manager, result.start, EPSILON_SYMBOL, result.end);
    add_transition(manager, a->end, EPSILON_SYMBOL, result.end);

    return result;
}


/**
 * @brief Converts a postfix regular expression into an NFA.
 *
 * Uses Thompson's construction algorithm.
 * The regex is processed using a stack of NFA fragments.
 *
 * @param r Postfix regular expression.
 * @return Fully constructed NFA.
 */
nfa regex_to_nfa(const regex r)
{
    states_manager manager = new_states_manager();

    t_nfa stack[MAX_STATES];
    int top = -1;

    for (int i = 0; i < r.size; i++) {

        item current = r.items[i];

        switch (current.type) {

        case OPERAND:
            stack[++top] = symbol_nfa(&manager, current.value);
            break;

        case CONCATENATION: {
            t_nfa b = stack[top--];
            t_nfa a = stack[top--];
            stack[++top] = concat_nfa(&manager, &a, &b);
            break;
        }

        case ALTERNATION: {
            t_nfa b = stack[top--];
            t_nfa a = stack[top--];
            stack[++top] = union_nfa(&manager, &a, &b);
            break;
        }

        case KLEENE_STAR: {
            t_nfa a = stack[top--];
            stack[++top] = kleene_closure_nfa(&manager, &a);
            break;
        }

        case POSITIVE_CLOSURE: {
            t_nfa a = stack[top--];
            stack[++top] = positive_closure_nfa(&manager, &a);
            break;
        }

        case OPTIONAL: {
            t_nfa a = stack[top--];
            stack[++top] = optional_nfa(&manager, &a);
            break;
        }

        default:
            break;
        }
    }

    t_nfa final_fragment = stack[top];

    return t_nfa_to_nfa(final_fragment, manager);
}


/**
 * @brief Converts a temporary Thompson NFA into the final NFA structure.
 *
 * Allocates and builds:
 *  - Transition table (bitmask-based)
 *  - Accept states bitmask
 *  - Epsilon-closures
 *
 * @param temp_nfa Final Thompson fragment.
 * @param manager States manager containing transitions and alphabet.
 * @return Fully constructed NFA.
 */
nfa t_nfa_to_nfa(t_nfa temp_nfa, states_manager manager)
{
    nfa automaton;

    automaton.start_state = temp_nfa.start;
    automaton.states = manager.states_count;
    automaton.accept_states = (1ULL << temp_nfa.end);
    automaton.alphabet = manager.manager_alphabet;

    uint8_t S = automaton.states;
    uint16_t A = automaton.alphabet.symbol_count;

    automaton.transitions = malloc(S * sizeof(uint64_t *));
    for (uint8_t i = 0; i < S; i++)
        automaton.transitions[i] = calloc(A, sizeof(uint64_t));

    for (uint16_t i = 0; i < manager.transitions_count; i++) {
        t_transition t = manager.transitions[i];
        int col = automaton.alphabet.char_to_col[(unsigned char)t.symbol];
        automaton.transitions[t.from_state][col] |= (1ULL << t.to_state);
    }

    automaton.epsilon_closures = malloc(S * sizeof(uint64_t));
    calculate_epsilon_closure(&automaton);

    return automaton;
}


/**
 * @brief Computes the epsilon-closure of a given state.
 *
 * Uses a bitmask-based DFS to find all states reachable
 * via epsilon transitions.
 *
 * @param automaton Pointer to the NFA.
 * @param state State to compute closure for.
 */
void epsilon_closure(nfa *automaton, uint8_t state)
{
    uint64_t closure = 0;
    uint64_t stack = (1ULL << state);

    int epsilon_col =
        automaton->alphabet.char_to_col[(unsigned char)EPSILON_SYMBOL];

    while (stack) {

        uint8_t s = __builtin_ctzll(stack);
        stack &= ~(1ULL << s);

        if (closure & (1ULL << s))
            continue;

        closure |= (1ULL << s);

        uint64_t next =
            automaton->transitions[s][epsilon_col];

        stack |= next;
    }

    automaton->epsilon_closures[state] = closure;
}


/**
 * @brief Computes epsilon-closures for all states.
 *
 * @param automaton Pointer to the NFA.
 */
void calculate_epsilon_closure(nfa *automaton)
{
    for (uint8_t i = 0; i < automaton->states; i++)
        epsilon_closure(automaton, i);
}


/**
 * @brief Simulates the NFA on a given input string.
 *
 * Uses bitmask state sets and precomputed epsilon-closures.
 *
 * Algorithm:
 *  1. Start from epsilon-closure of start state.
 *  2. For each input character:
 *      - Compute reachable states.
 *      - Expand using epsilon-closures.
 *  3. Check if any accept state is reached.
 *
 * @param automaton The NFA to simulate.
 * @param input Input string.
 * @param input_length Length of the input.
 * @return true if the input matches the regex, false otherwise.
 */
bool match_nfa(nfa automaton,
               const char *input,
               size_t input_length)
{
    uint64_t current =
        automaton.epsilon_closures[automaton.start_state];

    for (size_t i = 0; i < input_length; i++) {

        char c = input[i];
        int col =
            automaton.alphabet.char_to_col[(unsigned char)c];

        if (col == -1)
            return false;

        uint64_t next = 0;

        uint64_t tmp = current;

        while (tmp) {
            uint8_t s = __builtin_ctzll(tmp);
            tmp &= ~(1ULL << s);
            next |= automaton.transitions[s][col];
        }

        uint64_t expanded = 0;
        tmp = next;

        while (tmp) {
            uint8_t s = __builtin_ctzll(tmp);
            tmp &= ~(1ULL << s);
            expanded |= automaton.epsilon_closures[s];
        }

        current = expanded;

        if (!current)
            return false;
    }

    return (current & automaton.accept_states) != 0;
}