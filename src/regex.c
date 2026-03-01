#include "regex.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief Returns the precedence level of a regex operator.
 *
 * Higher values indicate higher precedence.
 *
 * Precedence order (highest to lowest):
 *   1. Unary operators (*, +, ?)
 *   2. Concatenation
 *   3. Alternation (|)
 *
 * @param type The item type (operator).
 * @return Integer representing precedence level.
 */
static int get_precedence(item_type type)
{
    switch (type)
    {
        case KLEENE_STAR:
        case POSITIVE_CLOSURE:
        case OPTIONAL:
            return 3;

        case CONCATENATION:
            return 2;

        case ALTERNATION:
            return 1;

        default:
            return 0;
    }
}


/**
 * @brief Determines whether an explicit concatenation operator
 *        should be inserted between two items.
 *
 * Concatenation is required when:
 *   - The left item can end an expression
 *   - The right item can start an expression
 *
 * @param current Left item.
 * @param next Right item.
 * @return 1 if concatenation should be inserted, 0 otherwise.
 */
static int should_insert_concat(item current, item next)
{
    int left =
        current.type == OPERAND ||
        current.type == R_PARENTHESIS ||
        current.type == KLEENE_STAR ||
        current.type == POSITIVE_CLOSURE ||
        current.type == OPTIONAL;

    int right =
        next.type == OPERAND ||
        next.type == L_PARENTHESIS;

    return left && right;
}

/**
 * @brief Creates a new regex item.
 *
 * @param value Character value of the item.
 * @param type Type of the item.
 * @return Initialized item structure.
 */
item new_item(char value, item_type type)
{
    item it;
    it.value = value;
    it.type  = type;
    return it;
}


/**
 * @brief Determines the type of a character in a regex.
 *
 * Maps special characters to their corresponding
 * operator types. All other characters are treated as operands.
 *
 * @param c Character to classify.
 * @return Corresponding item_type.
 */
item_type get_item_type(char c)
{
    switch (c)
    {
        case KLEENE_STAR_SYMBOL:
            return KLEENE_STAR;

        case POSITIVE_CLOSURE_SYMBOL:
            return POSITIVE_CLOSURE;

        case OPTIONAL_SYMBOL:
            return OPTIONAL;

        case CONCAT_SYMBOL:
            return CONCATENATION;

        case ALTERNATION_SYMBOL:
            return ALTERNATION;

        case LEFT_PARENTHESIS_SYMBOL:
            return L_PARENTHESIS;

        case RIGHT_PARENTHESIS_SYMBOL:
            return R_PARENTHESIS;

        default:
            return OPERAND;
    }
}


/**
 * @brief Converts a regex string into an array of items (tokens).
 *
 * Handles escape sequences by treating the escaped character
 * as a literal operand.
 *
 * Example:
 *   Input:  "a\*b"
 *   Output: 'a', '*', 'b' (where '*' is treated as operand)
 *
 * @param regex_str Input regex string.
 * @param out_size Output parameter storing number of items.
 * @return Dynamically allocated array of items, or NULL on failure.
 *
 * @note Caller is responsible for freeing the returned array.
 */
item *itemize_regex(const char *regex_str, int *out_size)
{
    if (!regex_str || !out_size)
        return NULL;

    size_t len = strlen(regex_str);
    item *items = malloc(sizeof(item) * len);
    if (!items)
        return NULL;

    int count = 0;

    for (size_t i = 0; i < len; i++)
    {
        char c = regex_str[i];

        if (c == ESCAPE_SYMBOL && i + 1 < len)
        {
            items[count++] = new_item(regex_str[++i], OPERAND);
        }
        else
        {
            items[count++] = new_item(c, get_item_type(c));
        }
    }

    *out_size = count;
    return items;
}


/**
 * @brief Converts implicit concatenations into explicit ones.
 *
 * For example:
 *   "ab"   →  "a.b"
 *   "a(b)" →  "a.(b)"
 *
 * The function inserts CONCAT_SYMBOL where needed.
 *
 * @param items Input array of items.
 * @param size Number of input items.
 * @param out_size Output parameter storing result size.
 * @return New dynamically allocated array with explicit concatenation.
 *
 * @note Caller must free the returned array.
 */
item *implicit_to_explicit_concatenation(const item *items,
                                         int size,
                                         int *out_size)
{
    if (!items || size <= 0 || !out_size)
        return NULL;

    item *result = malloc(sizeof(item) * size * 2);
    if (!result)
        return NULL;

    int count = 0;

    for (int i = 0; i < size; i++)
    {
        result[count++] = items[i];

        if (i < size - 1 &&
            should_insert_concat(items[i], items[i + 1]))
        {
            result[count++] =
                new_item(CONCAT_SYMBOL, CONCATENATION);
        }
    }

    *out_size = count;
    return result;
}


/**
 * @brief Converts an infix regex expression to postfix notation.
 *
 * Implements Dijkstra's Shunting Yard algorithm.
 *
 * Handles:
 *   - Operator precedence
 *   - Parentheses grouping
 *   - Unary and binary operators
 *
 * Returns NULL if parentheses are mismatched.
 *
 * @param items Input infix items.
 * @param size Number of input items.
 * @param out_size Output parameter storing postfix size.
 * @return Dynamically allocated postfix array, or NULL on error.
 *
 * @note Caller must free the returned array.
 */
item *shunting_yard(const item *items,
                    int size,
                    int *out_size)
{
    if (!items || size <= 0 || !out_size)
        return NULL;

    item *output = malloc(sizeof(item) * size);
    item *stack  = malloc(sizeof(item) * size);

    if (!output || !stack)
    {
        free(output);
        free(stack);
        return NULL;
    }

    int out_count = 0;
    int top = -1;

    for (int i = 0; i < size; i++)
    {
        item current = items[i];

        switch (current.type)
        {
            case OPERAND:
                output[out_count++] = current;
                break;

            case KLEENE_STAR:
            case POSITIVE_CLOSURE:
            case OPTIONAL:
            case CONCATENATION:
            case ALTERNATION:

                while (top >= 0 &&
                       stack[top].type != L_PARENTHESIS &&
                       get_precedence(stack[top].type) >=
                       get_precedence(current.type))
                {
                    output[out_count++] = stack[top--];
                }

                stack[++top] = current;
                break;

            case L_PARENTHESIS:
                stack[++top] = current;
                break;

            case R_PARENTHESIS:

                while (top >= 0 &&
                       stack[top].type != L_PARENTHESIS)
                {
                    output[out_count++] = stack[top--];
                }

                if (top < 0)
                {
                    free(output);
                    free(stack);
                    return NULL; // Mismatched parentheses
                }

                top--; // Remove '('
                break;
        }
    }

    while (top >= 0)
    {
        if (stack[top].type == L_PARENTHESIS)
        {
            free(output);
            free(stack);
            return NULL;
        }

        output[out_count++] = stack[top--];
    }

    free(stack);

    *out_size = out_count;
    return output;
}


/**
 * @brief Parses a regular expression string into postfix form.
 *
 * This function executes the complete pipeline:
 *
 *   1. Tokenization
 *   2. Insert explicit concatenation
 *   3. Convert infix to postfix (Shunting Yard)
 *
 * If any step fails, an empty regex structure is returned.
 *
 * @param regex_str Input regular expression string.
 * @return Parsed regex structure in postfix form.
 *
 * @note The returned regex must be freed using free_regex().
 */
regex parse_regex(const char *regex_str)
{
    regex result;
    result.items = NULL;
    result.size = 0;

    if (!regex_str)
        return result;

    int size1, size2, size3;

    item *step1 = itemize_regex(regex_str, &size1);
    if (!step1)
        return result;

    item *step2 =
        implicit_to_explicit_concatenation(step1, size1, &size2);

    free(step1);

    if (!step2)
        return result;

    item *step3 =
        shunting_yard(step2, size2, &size3);

    free(step2);

    if (!step3)
        return result;

    result.items = step3;
    result.size  = size3;

    return result;
}


/**
 * @brief Frees memory allocated for a regex structure.
 *
 * @param r Regex structure to free.
 */
void free_regex(regex r)
{
    free(r.items);
}