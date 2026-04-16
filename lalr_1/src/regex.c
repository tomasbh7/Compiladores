#include "regex.h"
#include <string.h>
#include <stdlib.h>

item *itemize_regex(const char *regex_str, int *out_size);
item *shunting_yard(const item *items, int size, int *out_size);
item *implicit_to_explicit_concatenation(const item *items, int size, int *out_size);
item new_item(char value, item_type type);
item_type get_item_type(char c);

regex parse_regex(const char *regex_str)
{
    // First, we itemize the regex string into an array of items
    int size;
    item *items = itemize_regex(regex_str, &size);
    // Next, we convert implicit concatenation to explicit concatenation
    int explicit_size;
    item *explicit_items = implicit_to_explicit_concatenation(items, size, &explicit_size);
    // We can free the original items array as we no longer need it
    free(items);
    // Finally, we convert the infix notation to postfix notation using the shunting yard algorithm
    int postfix_size = 0;
    item *postfix_items = shunting_yard(explicit_items, explicit_size, &postfix_size);
    // We can free the explicit items array as we no longer need it
    free(explicit_items);

    regex result;
    result.size = postfix_size;
    result.items = postfix_items;

    return result;
}

/**
 * @brief Function to convert a regex string into an array of items.
 * This function iterates through the input regex string, creates items
 * for each character, and determines if they are operators or not.
 *
 * @param regex_str The input regular expression as a string
 * @param out_size A pointer to an integer where the size of the output array will be stored
 * @return An array of items representing the regex
 */
item *itemize_regex(const char *regex_str, int *out_size)
{
    // Get an approximation of the number of items
    // in the regex by counting the characters
    int length = strlen(regex_str);

    // Create a temporary array to hold the items
    item temp_items[length];

    *out_size = 0;

    // Iterate through the regex string and create items
    for (size_t i = 0; i < length; i++)
    {
        if (regex_str[i] == ESCAPE_SYMBOL && i + 1 < length)
        {
            // If we encounter an escape symbol, we need to skip
            // it and take the next character as a literal
            temp_items[(*out_size)++] = new_item(regex_str[++i], OPERAND);
        }
        else
        {
            // For other characters, we check if they are operators or not
            temp_items[(*out_size)++] = new_item(regex_str[i], get_item_type(regex_str[i]));
        }
    }

    // Allocate memory for the items array in the regex struct
    item *items = malloc(*out_size * sizeof(item));

    // Copy items from the temporary array to the allocated array
    for (int i = 0; i < *out_size; i++)
    {
        items[i] = temp_items[i];
    }

    return items;
}

/**
 * @brief Function to convert an array of items from infix notation to postfix notation
 * using the Shunting Yard algorithm. This function handles operator precedence and
 * ensures that the output is in the correct order for further processing.
 *
 * @param items The input array of items in infix notation
 * @param size The number of items in the input array
 * @param out_size Pointer to an int where the size of the output array will be stored
 * @return An array of items in postfix notation, or NULL if there are mismatched parentheses
 */
item *shunting_yard(const item *items, int size, int *out_size)
{
    // Shunting Yard algorithm initialization
    item operators[size];
    item output[size];
    int operators_top = -1;
    int output_top = -1;

    // Iterate through the items and apply the shunting yard algorithm
    for (size_t i = 0; i < size; i++)
    {
        if (items[i].type == OPERAND)
        {
            // If the item is not an operator, add it to the output
            output[++output_top] = items[i];
        }
        else
        {
            // If the item is a right parenthesis, pop operators to the output
            // until we find a left parenthesis. If we don't find a left parenthesis,
            // it means there are mismatched parentheses.
            if (items[i].type == R_PARENTHESIS)
            {
                while (operators_top != -1 && operators[operators_top].type != L_PARENTHESIS)
                {
                    output[++output_top] = operators[operators_top--];
                }
                if (operators_top == -1)
                {
                    // Mismatched parentheses
                    return NULL;
                }
                // Pop the left parenthesis from the stack
                operators_top--;
            }
            else if (items[i].type == L_PARENTHESIS)
            {
                operators[++operators_top] = items[i];
            }
            else
            {
                // If the item is an operator, pop operators from the stack to the output
                // while they have higher or equal precedence and are not left parentheses
                while (operators_top != -1 &&
                       operators[operators_top].type >= items[i].type &&
                       operators[operators_top].type != L_PARENTHESIS)
                {
                    output[++output_top] = operators[operators_top--];
                }
                operators[++operators_top] = items[i];
            }
        }
    }

    // After processing all items, pop any remaining operators from the stack to the output
    while (operators_top != -1)
    {
        if (operators[operators_top].type == L_PARENTHESIS)
        {
            // Mismatched parentheses
            return NULL;
        }
        output[++output_top] = operators[operators_top--];
    }

    // Clean up the output array to return only the valid items
    item *result = malloc((output_top + 1) * sizeof(item));
    for (int i = 0; i <= output_top; i++)
    {
        result[i] = output[i];
    }
    if (out_size)
    {
        *out_size = output_top + 1;
    }
    return result;
}

/**
 * @brief Function to convert implicit concatenation in the regex to explicit concatenation.
 * For example, "ab" will be converted to "a.b". This function iterates through the items
 * and inserts a concatenation operator where needed based on the types of adjacent items.
 *
 * @param items The input array of items with potential implicit concatenation
 * @param size The number of items in the input array
 * @param out_size A pointer to an integer where the size of the output array will be stored
 * @return An array of items with explicit concatenation operators, and the size of the output array
 */
item *implicit_to_explicit_concatenation(const item *items, int size, int *out_size)
{
    // This function will convert implicit concatenation in the regex to explicit concatenation
    // For example, "ab" will be converted to "a.b"

    // We can have at most size * 2 items in the worst case (every item is an operand followed by a concatenation operator)
    item temp_items[size * 2];
    int temp_size = 0;

    // Iterate through the items and insert concatenation operators where needed
    for (size_t i = 0; i < size; i++)
    {
        temp_items[temp_size++] = items[i];

        // Check if we need to insert a concatenation operator
        if (i + 1 < size &&
            ((items[i].type == OPERAND || items[i].type == R_PARENTHESIS || items[i].type == KLEENE_STAR || items[i].type == POSITIVE_CLOSURE || items[i].type == OPTIONAL) &&
             (items[i + 1].type == OPERAND || items[i + 1].type == L_PARENTHESIS)))
        {
            temp_items[temp_size++] = new_item(CONCATENATION_SYMBOL, CONCATENATION);
        }
    }

    // Allocate memory for the output array
    item *result = malloc(temp_size * sizeof(item));
    for (int i = 0; i < temp_size; i++)
    {
        result[i] = temp_items[i];
    }

    *out_size = temp_size;
    return result;
}

/**
 * @brief Helper function to create a new item with the given value and operator status.
 * This function initializes an item struct with the provided character value and item type.
 * @param value The character value of the item
 * @param type An item_type indicating the type of the operator or operand
 * @return An item struct initialized with the given value and type
 */
item new_item(char value, item_type type)
{
    item new_item;
    new_item.value = value;
    new_item.type = type;
    return new_item;
}

/**
 * @brief Helper function to determine the type of an item based on its character value.
 *
 * @param c The character to check
 * @return An item_type corresponding to the character, or OPERAND if it's not an operator
 */
item_type get_item_type(char c)
{
    switch (c)
    {
    case LEFT_PARENTHESIS_SYMBOL:
        return L_PARENTHESIS;
    case RIGHT_PARENTHESIS_SYMBOL:
        return R_PARENTHESIS;
    case CONCATENATION_SYMBOL:
        return CONCATENATION;
    case KLEENE_STAR_SYMBOL:
        return KLEENE_STAR;
    case POSITIVE_CLOSURE_SYMBOL:
        return POSITIVE_CLOSURE;
    case OPTIONAL_SYMBOL:
        return OPTIONAL;
    case ALTERNATION_SYMBOL:
        return ALTERNATION;
    default:
        return OPERAND;
    }
}

void free_regex(regex r)
{
    if (r.items)
    {
        free(r.items);
    }
}