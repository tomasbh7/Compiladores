#include "nfa.h"

/**
 * @brief Struct to represent a transition in the NFA. It contains the source state,
 * the symbol for the transition, and the destination state. This struct is used as
 * an intermediate representation of transitions while building the NFA.
 */
struct temp_transition
{
    /* Represents a transition in the NFA */
    uint8_t from_state;
    /* The symbol for the transition */
    char symbol;
    /* The state to which the transition leads */
    uint8_t to_state;
};
typedef struct temp_transition t_transition;

/**
 * @brief Struct to represent a temporary NFA during construction. It contains the start
 * state and the end state. This struct is used as an intermediate representation while
 * building the NFA from the regex.
 */
struct temp_nfa
{
    /* Start state of the temporary NFA */
    uint8_t start;
    /* End state of the temporary NFA */
    uint8_t end;
};
typedef struct temp_nfa t_nfa;

/**
 * @brief Struct to manage states and transitions during NFA construction. It keeps track
 * of the next available state ID, the list of states, the list of transitions, and the
 * alphabet used by the NFAs being constructed.
 */
struct states_manager
{
    /* Next available state ID */
    uint8_t next_id;

    /* List of states managed */
    uint8_t states[MAX_STATES];
    uint8_t states_count;

    /* List of transitions managed */
    t_transition transitions[MAX_STATES * MAX_STATES];
    uint8_t transitions_count;

    /* Alphabet used by the states manager */
    alphabet manager_alphabet;
};
typedef struct states_manager states_manager;

// Function prototypes for internal helper functions

void epsilon_closure(nfa *automaton, uint8_t state);
void calculate_epsilon_closure(nfa *automaton);
nfa t_nfa_to_nfa(t_nfa temp_nfa, states_manager manager);

/**
 * @brief Function to create a new alphabet. This function initializes an alphabet struct with
 * default values, including setting the epsilon symbol and initializing the character-to-column
 * mapping.
 */
alphabet new_alphabet()
{
    alphabet a;

    memset(a.char_to_col, -1, sizeof(a.char_to_col));
    memset(a.symbols, 0, sizeof(a.symbols));

    // Add the epsilon symbol to the alphabet
    // For mapping purposes, the epsilon symbol is at index 0
    a.symbols[0] = EPSILON_SYMBOL;
    a.char_to_col[EPSILON_SYMBOL] = 0;

    a.symbol_count = 1;
    return a;
}

/**
 * @brief Function to add a symbol to the alphabet. This function checks if the symbol is already
 * in the alphabet, and if not, it adds the symbol to the symbols array and updates the
 * character-to-column mapping.
 * @param a Pointer to the alphabet struct to which the symbol should be added
 * @param symbol The symbol to add to the alphabet
 */
void add_symbol(alphabet *a, char symbol)
{
    // Check if the symbol is already in the alphabet
    if (a->char_to_col[(unsigned char)symbol] != -1 || symbol == EPSILON_SYMBOL)
    {
        return; // Symbol already exists, do nothing
    }

    // Add the symbol to the alphabet
    a->symbols[a->symbol_count] = symbol;
    a->char_to_col[(unsigned char)symbol] = a->symbol_count;
    a->symbol_count++;
}

/**
 * @brief Function to create a new states manager. This function initializes a states_manager struct
 * with default values, including setting the next available state ID to 0 and initializing the
 * alphabet.
 * @return A new states_manager struct initialized with default values
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
 * @brief Function to create a new state in the states manager. This function assigns a new state ID,
 * adds it to the list of states, and returns the new state ID.
 * @param manager Pointer to the states_manager struct that manages the states
 * @return The ID of the newly created state
 */
uint8_t new_state(states_manager *manager)
{
    uint8_t state = manager->next_id;
    manager->states[manager->states_count] = state;
    manager->states_count++;
    manager->next_id++;

    return state;
}

/**
 * @brief Function to add a transition to the states manager. This function creates a new transition
 * struct and adds it to the list of transitions.
 * @param manager Pointer to the states_manager struct that manages the states and transitions
 * @param from_state The state from which the transition originates
 * @param symbol The symbol on which the transition occurs
 * @param to_state The state to which the transition leads
 */
void add_transition(states_manager *manager, uint8_t from_state, char symbol, uint8_t to_state)
{
    t_transition transition;
    transition.from_state = from_state;
    transition.symbol = symbol;
    transition.to_state = to_state;

    manager->transitions[manager->transitions_count] = transition;
    manager->transitions_count++;
}

/**
 * @brief Function to create a new NFA that represents the concatenation of two NFAs. This function
 * takes two NFAs as input, creates a new NFA that has the start state of the first NFA and the end
 * state of the second NFA, and adds an epsilon transition from the end state of the first NFA to
 * the start state of the second NFA.
 * @param manager Pointer to the states_manager struct that manages the states and transitions
 * @param a Pointer to the first NFA
 * @param b Pointer to the second NFA
 * @return A new NFA struct representing the concatenation of the two input NFAs
 */
t_nfa concat_nfa(states_manager *manager, t_nfa *a, t_nfa *b)
{
    // Create a new nfa that represents the concatenation of a and b.
    t_nfa result;

    // The start state of the result is the start state of a
    result.start = a->start;
    // The end state of the result is the end state of b
    result.end = b->end;

    // Add an epsilon transition from the end state of a to the start state of b
    add_transition(manager, a->end, EPSILON_SYMBOL, b->start);

    return result;
}

/**
 * @brief Function to create a new NFA that represents a single symbol. This function creates a new NFA
 * with a start state and an end state, and adds a transition from the start state to the end state on
 * the given symbol.
 * @param manager Pointer to the states_manager struct that manages the states and transitions
 * @param symbol The symbol for which the NFA should be created
 * @return A new NFA struct representing the given symbol
 */
t_nfa symbol_nfa(states_manager *manager, char symbol)
{
    // Create a new nfa that represents the symbol.
    t_nfa result;

    // The start state of the result is the next available state
    result.start = new_state(manager);

    // The end state of the result is the next available state
    result.end = new_state(manager);

    // Add a transition from the start state to the end state on the given symbol
    add_transition(manager, result.start, symbol, result.end);

    return result;
}

/**
 * @brief Function to create a new NFA that represents the union of two NFAs. This function takes two NFAs
 * as input, creates a new NFA that has a new start state and a new end state, and adds epsilon transitions
 * from the new start state to the start states of both input NFAs, and from the end states of both input
 * NFAs to the new end state.
 * @param manager Pointer to the states_manager struct that manages the states and transitions
 * @param a Pointer to the first NFA
 * @param b Pointer to the second NFA
 * @return A new NFA struct representing the union of the two input NFAs
 */
t_nfa union_nfa(states_manager *manager, t_nfa *a, t_nfa *b)
{
    // Create a new nfa that represents the union of a and b.
    t_nfa result;

    // The start state of the result is the next available state
    result.start = new_state(manager);

    // The end state of the result is the next available state
    result.end = new_state(manager);

    // Add an epsilon transition from the start state of the result to the start state of a
    add_transition(manager, result.start, EPSILON_SYMBOL, a->start);

    // Add an epsilon transition from the start state of the result to the start state of b
    add_transition(manager, result.start, EPSILON_SYMBOL, b->start);

    // Add an epsilon transition from the end state of a to the end state of the result
    add_transition(manager, a->end, EPSILON_SYMBOL, result.end);

    // Add an epsilon transition from the end state of b to the end state of the result
    add_transition(manager, b->end, EPSILON_SYMBOL, result.end);

    return result;
}

/**
 * @brief Function to create a new NFA that represents the positive closure of an NFA.
 * This function takes an NFA as input, creates a new NFA that has a new start state and
 * a new end state, and adds epsilon transitions from the new start state to the start
 * state of the input NFA, from the end state of the input NFA to the start state of the
 * input NFA, and from the end state of the input NFA to the new end state.
 * @param manager Pointer to the states_manager struct that manages the states and transitions
 * @param a Pointer to the input NFA
 * @return A new NFA struct representing the positive closure of the input NFA
 */
t_nfa positive_closure_nfa(states_manager *manager, t_nfa *a)
{
    // Create a new nfa that represents the positive closure of a.
    t_nfa result;

    // The start state of the result is the next available state
    result.start = new_state(manager);

    // The end state of the result is the next available state
    result.end = new_state(manager);

    // Add an epsilon transition from the start state of the result to the start state of a
    add_transition(manager, result.start, EPSILON_SYMBOL, a->start);

    // Add an epsilon transition from the end state of a to the start state of a
    add_transition(manager, a->end, EPSILON_SYMBOL, a->start);

    // Add an epsilon transition from the end state of a to the end state of the result
    add_transition(manager, a->end, EPSILON_SYMBOL, result.end);

    return result;
}


/**
 * @brief Function to create a new NFA that represents the Kleene closure of an NFA. This function
 * takes an NFA as input, creates a new NFA that has a new start state and a new end state, and adds
 * epsilon transitions from the new start state to the start state of the input NFA, from
 * the end state of the input NFA to the start state of the input NFA, from the end state of the input
 * NFA to the new end state, and from the new start state to the new end state (to allow for the empty
 * string).
 * @param manager Pointer to the states_manager struct that manages the states and transitions
 * @param a Pointer to the input NFA
 * @return A new NFA struct representing the Kleene closure of the input NFA
 */
t_nfa kleene_closure_nfa(states_manager *manager, t_nfa *a)
{
    // Create a new postive closure nfa for a
    t_nfa result = positive_closure_nfa(manager, a);

    // Add an epsilon transition from the start state of the result to the end state of the result
    add_transition(manager, result.start, EPSILON_SYMBOL, result.end);

    return result;
}

/**
 * @brief Function to create a new NFA that represents the optionality of an NFA. This function takes
 * an NFA as input, creates a new NFA that adds epsilon transitions from the start state of the input
 * NFA to the end state (to allow for the empty string).
 * @param manager Pointer to the states_manager struct that manages the states and transitions
 * @param a Pointer to the input NFA
 * @return A new NFA struct representing the optionality of the input NFA
 */
t_nfa optional_nfa(states_manager *manager, t_nfa *a)
{
    // Add an epsilon transition from the start state of a to the end state of a
    add_transition(manager, a->start, EPSILON_SYMBOL, a->end);

    return *a;
}


nfa regex_to_nfa(const regex r)
{
    // Create a new states manager
    states_manager manager = new_states_manager();

    // Initialize a stack to hold the uint8_termediate NFAs
    t_nfa stack[MAX_STATES];
    uint8_t stack_top = -1;

    // Process each item in the regex
    for (uint8_t i = 0; i < r.size; i++)
    {
        // Get the current item
        item current_item = r.items[i];

        // If the item is an operand, create a new NFA for the symbol and push it onto the stack
        if (current_item.type == OPERAND)
        {
            stack[++stack_top] = symbol_nfa(&manager, current_item.value);
            // Add the symbol to the manager's alphabet
            add_symbol(&manager.manager_alphabet, current_item.value);
        }
        // Else, the item is an operator, so pop the necessary NFAs from the stack, apply the
        // operator, and push the result back onto the stack
        else
        {
            if (current_item.type == CONCATENATION)
            {
                t_nfa b = stack[stack_top--];
                t_nfa a = stack[stack_top--];
                stack[++stack_top] = concat_nfa(&manager, &a, &b);
            }
            else if (current_item.type == ALTERNATION)
            {
                t_nfa b = stack[stack_top--];
                t_nfa a = stack[stack_top--];
                stack[++stack_top] = union_nfa(&manager, &a, &b);
            }
            else if (current_item.type == POSITIVE_CLOSURE)
            {
                t_nfa a = stack[stack_top--];
                stack[++stack_top] = positive_closure_nfa(&manager, &a);
            }
            else if (current_item.type == KLEENE_STAR)
            {
                t_nfa a = stack[stack_top--];
                stack[++stack_top] = kleene_closure_nfa(&manager, &a);
            }
            else if (current_item.type == OPTIONAL)
            {
                t_nfa a = stack[stack_top--];
                stack[++stack_top] = optional_nfa(&manager, &a);
            }
        }
    }

    // The final NFA is the only NFA left on the stack
    if (stack_top != 0)
    {
        fprintf(stderr, "Error: Invalid regex. Stack should have exactly one NFA left, but has %d.\n", stack_top + 1);
        exit(EXIT_FAILURE);
    }
    else
    {
        t_nfa temp_nfa = stack[stack_top];
        return t_nfa_to_nfa(temp_nfa, manager);
    }
}

/**
 * @brief Function to convert a temporary NFA representation (t_nfa) into the final NFA struct. This function
 * takes the start and end states from the temporary NFA, initializes the transition table based on the
 * transitions stored in the states manager, and calculates the epsilon closures for all states.
 * @param temp_nfa The temporary NFA representation containing the start and end states
 * @param manager The states_manager struct that contains the transitions and alphabet information
 * @return An NFA struct representing the final non-deterministic finite automaton
 */
nfa t_nfa_to_nfa(t_nfa temp_nfa, states_manager manager)
{
    nfa result;
    result.start_state = temp_nfa.start;
    result.states = manager.states_count;
    result.accept_states = (1ULL << temp_nfa.end);
    result.nfa_alphabet = manager.manager_alphabet;

    // Initialize the transition table with empty sets
    result.transitions = malloc(result.states * sizeof(uint64_t *));
    for (int i = 0; i < result.states; i++)
    {
        result.transitions[i] = malloc(result.nfa_alphabet.symbol_count * sizeof(uint64_t));
        for (int j = 0; j < result.nfa_alphabet.symbol_count; j++)
            result.transitions[i][j] = 0;
    }

    // Fill the transition table based on the transitions in the manager
    for (uint8_t i = 0; i < manager.transitions_count; i++)
    {
        t_transition t = manager.transitions[i];
        int col = result.nfa_alphabet.char_to_col[(unsigned char)t.symbol];
        result.transitions[t.from_state][col] |= (1ULL << t.to_state);
    }

    calculate_epsilon_closure(&result);

    return result;
}

/**
 * @brief Function to calculate the epsilon closure for all states in the given NFA.
 * This function initializes a cache to store the epsilon closures and computes the closure
 * for each state using a depth-first search approach.
 * @param automaton Pointer to the NFA for which epsilon closures are to be calculated
 */
void calculate_epsilon_closure(nfa *automaton)
{
    // Allocate cache storage sized for all states.
    uint64_t *closure_cache = malloc(automaton->states * sizeof(uint64_t));

    // Initialize the cache to 0 so we can detect "not computed".
    for (int i = 0; i < automaton->states; i++)
    {
        closure_cache[i] = 0;
    }

    // Attach the cache to the automaton.
    automaton->epsilon_closure_cache = closure_cache;

    // Compute all closures up front so lookups are O(1).
    for (uint8_t state = 0; state < automaton->states; state++)
    {
        epsilon_closure(automaton, state);
    }
}

/**
 * @brief Function to compute the epsilon closure for a given state in the NFA.
 * This function uses a depth-first search approach to find all states reachable
 * from the given state via epsilon transitions.
 * @param automaton Pointer to the NFA
 * @param state The state for which the epsilon closure is to be computed
 */
void epsilon_closure(nfa *automaton, uint8_t state)
{
    // Return if the closure for this state has already been computed and cached.
    if (automaton->epsilon_closure_cache[state] != 0)
        return;

    // Start with the state itself in the closure.
    uint64_t closure = (1ULL << state);

    // Use an explicit stack for DFS over epsilon transitions.
    uint8_t stack[MAX_STATES];
    int stack_top = -1;

    // Push the initial state.
    stack[++stack_top] = state;

    // Explore epsilon transitions until the stack is empty.
    while (stack_top >= 0)
    {
        uint8_t current_state = stack[stack_top--];
        uint64_t epsilon_transitions = automaton->transitions[current_state][0];

        // Add newly discovered states to the closure and stack.
        for (uint8_t next_state = 0; next_state < automaton->states; next_state++)
        {
            if ((epsilon_transitions & (1ULL << next_state)) != 0 &&
                (closure & (1ULL << next_state)) == 0)
            {
                closure |= (1ULL << next_state);
                stack[++stack_top] = next_state;
            }
        }
    }

    // Cache the computed closure for this state.
    automaton->epsilon_closure_cache[state] = closure;
}

bool match_nfa(nfa automaton, const char *input, size_t input_length)
{
    // Start with the epsilon closure of the start state.
    uint64_t current_states = automaton.epsilon_closure_cache[automaton.start_state];

    // Process each input character.
    for (size_t i = 0; i < input_length; i++)
    {
        char symbol = input[i];
        int col = automaton.nfa_alphabet.char_to_col[(unsigned char)symbol];

        // If the symbol is not in the alphabet, no transitions are possible.
        if (col == -1)
        {
            return false;
        }

        uint64_t next_states = 0;

        // For each current state, find reachable states on the input symbol.
        for (uint8_t state = 0; state < automaton.states; state++)
        {
            if ((current_states & (1ULL << state)) != 0)
            {
                next_states |= automaton.transitions[state][col];
            }
        }

        // Compute the epsilon closure of the next states.
        uint64_t new_current_states = 0;
        for (uint8_t state = 0; state < automaton.states; state++)
        {
            if ((next_states & (1ULL << state)) != 0)
            {
                new_current_states |= automaton.epsilon_closure_cache[state];
            }
        }

        current_states = new_current_states;

        // If there are no current states, the input is rejected.
        if (current_states == 0)
        {
            return false;
        }
    }

    // Check if any of the current states are accept states.
    return (current_states & automaton.accept_states) != 0;
}

bool save_nfa(const nfa *automaton, const char *file_path)
{
    if (automaton == NULL || file_path == NULL)
    {
        return false;
    }

    FILE *file = fopen(file_path, "wb");
    if (file == NULL)
    {
        return false;
    }

    const uint32_t magic = 0x3141464E; // NFA1
    const uint32_t version = 1;
    const int32_t symbol_count = automaton->nfa_alphabet.symbol_count;

    if (symbol_count <= 0 || symbol_count > 256)
    {
        fclose(file);
        return false;
    }

    if (fwrite(&magic, sizeof(magic), 1, file) != 1 ||
        fwrite(&version, sizeof(version), 1, file) != 1 ||
        fwrite(&automaton->start_state, sizeof(automaton->start_state), 1, file) != 1 ||
        fwrite(&automaton->states, sizeof(automaton->states), 1, file) != 1 ||
        fwrite(&automaton->accept_states, sizeof(automaton->accept_states), 1, file) != 1 ||
        fwrite(&symbol_count, sizeof(symbol_count), 1, file) != 1)
    {
        fclose(file);
        return false;
    }

    if (fwrite(automaton->nfa_alphabet.symbols, sizeof(char), (size_t)symbol_count, file) != (size_t)symbol_count)
    {
        fclose(file);
        return false;
    }

    for (uint8_t state = 0; state < automaton->states; state++)
    {
        if (fwrite(automaton->transitions[state], sizeof(uint64_t), (size_t)symbol_count, file) != (size_t)symbol_count)
        {
            fclose(file);
            return false;
        }
    }

    if (fclose(file) != 0)
    {
        return false;
    }

    return true;
}

void free_nfa(nfa *automaton)
{
    if (automaton == NULL)
    {
        return;
    }

    if (automaton->transitions != NULL)
    {
        for (uint8_t state = 0; state < automaton->states; state++)
        {
            free(automaton->transitions[state]);
        }
        free(automaton->transitions);
    }

    free(automaton->epsilon_closure_cache);

    automaton->transitions = NULL;
    automaton->epsilon_closure_cache = NULL;
    automaton->states = 0;
    automaton->start_state = 0;
    automaton->accept_states = 0;
}