#ifndef REGEX_H
#define REGEX_H

/**
 * @brief Explicit concatenation symbol.
 */
#define CONCAT_SYMBOL '.'     

/**
 * @brief Kleene star operator (*).
 * Represents zero or more repetitions.
 */
#define KLEENE_STAR_SYMBOL '*'

/**
 * @brief Alternation operator (|).
 * Represents logical OR between expressions.
 */
#define ALTERNATION_SYMBOL '|' 

/**
 * @brief Left parenthesis symbol.
 */
#define LEFT_PARENTHESIS_SYMBOL '('

/**
 * @brief Right parenthesis symbol.
 */
#define RIGHT_PARENTHESIS_SYMBOL ')'

/**
 * @brief Positive closure operator (+).
 * Represents one or more repetitions.
 */
#define POSITIVE_CLOSURE_SYMBOL '+' 

/**
 * @brief Optional operator (?).
 * Represents zero or one occurrence.
 */
#define OPTIONAL_SYMBOL '?'         

/**
 * @brief Escape character.
 *
 * When this symbol appears in the regex, the next character
 * is treated as a literal operand instead of an operator.
 */
#define ESCAPE_SYMBOL '\\'    // Carácter de escape

/**
 * @brief Represents the different types of tokens in a regular expression.
 *
 * The enum values are ordered according to operator precedence
 * (lower numeric value = higher precedence).
 */
typedef enum
{
    /**
     * @brief Kleene star (*)
     * Zero or more repetitions.
     */
    KLEENE_STAR = 0,

    /**
     * @brief Positive closure (+)
     * One or more repetitions.
     */
    POSITIVE_CLOSURE,

    /**
     * @brief Optional (?)
     * Zero or one occurrence.
     */
    OPTIONAL,

    /**
     * @brief Explicit concatenation operator (.).
     */
    CONCATENATION,

    /**
     * @brief Alternation operator (|).
     */
    ALTERNATION,

    /**
     * @brief Literal character (operand).
     */
    OPERAND,

    /**
     * @brief Left parenthesis '('.
     */
    L_PARENTHESIS,
    
    /**
     * @brief Right parenthesis ')'.
     */
    R_PARENTHESIS

} item_type;


/**
 * @brief Represents a single token (item) in a regular expression.
 *
 * An item may be:
 *  - An operator
 *  - A literal operand
 *  - A parenthesis
 */
typedef struct {
    char value;       /**< Character value of the token */
    item_type type;  /**< Type of token */
} item;


/**
 * @brief Main structure representing a parsed regular expression.
 *
 * The regex is stored in:
 *  - Postfix notation
 *  - With explicit concatenation operators
 *
 * This format is ideal for Thompson's NFA construction.
 */
typedef struct {
    item* items;      /**< Array of items in postfix notation */
    int size;        /**< Number of items */
} regex;


/**
 * @brief Parses a regular expression string.
 *
 * Processing steps:
 *   1. Tokenizes the input string (handles escape characters)
 *   2. Converts implicit concatenation into explicit concatenation
 *   3. Converts infix notation to postfix notation (Shunting Yard algorithm)
 *
 * Example:
 *   Input:  "a(b|c)*"
 *   Output (postfix): "abc|*."
 *
 * @param regex_str Input regular expression string.
 * @return A regex structure containing the postfix representation.
 *
 * @note The returned structure must be freed using free_regex().
 */
regex parse_regex(const char *regex_str);


/**
 * @brief Frees memory associated with a regex structure.
 *
 * @param r Regex structure to free.
 */
void free_regex(regex r);


/**
 * @brief Prints the postfix representation of a regex.
 *
 * Intended for debugging and verification purposes.
 *
 * @param r Regex structure to print.
 */
void print_regex(regex r);

#endif // REGEX_H