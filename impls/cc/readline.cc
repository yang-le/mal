#include "readline.hh"
#include <readline/history.h>
#include <readline/readline.h>

static const Env *env_;

/* Generator function for mal command completion.  STATE lets us know whether
   to start from scratch; without any state (i.e. STATE == 0), then we
   start at the top of the list. */
static char *mal_command_generator(const char *text, int state)
{
    static size_t list_index, len;
    static std::vector<std::string> commands;
    const char *name;

    /* If this is a new word to complete, initialize now.  This includes
       saving the length of TEXT for efficiency, and initializing the index
       variable to 0. */
    if (!state)
    {
        list_index = 0;
        len = strlen(text);

        commands.clear();
        for (auto &[key, _] : *env_)
            commands.emplace_back(key);
    }

    /* Return the next name which partially matches from the command list. */
    while (list_index < commands.size())
    {
        name = commands[list_index++].c_str();

        if (strncmp(name, text, len) == 0)
            return (strdup(name));
    }

    /* If no names matched, then return NULL. */
    return ((char *)NULL);
}

/* Attempt to complete on the contents of TEXT.  START and END bound the
   region of rl_line_buffer that contains the word to complete.  TEXT is
   the word to complete.  We can use the entire contents of rl_line_buffer
   in case we want to do some simple parsing.  Return the array of matches,
   or NULL if there aren't any. */
static char **mal_completion(const char *text, int start, int end)
{
    return rl_completion_matches(text, mal_command_generator);
}

void initialize_readline(const Env *env)
{
    env_ = env;
    /* Tell the completer that we want a crack first. */
    rl_attempted_completion_function = mal_completion;
}

/* Read a string, and return a pointer to it.
   Returns NULL on EOF. */
char *rl_gets(const char *prompt)
{
    /* A static variable for holding the line. */
    static char *line_read = (char *)NULL;

    /* If the buffer has already been allocated,
       return the memory to the free pool. */
    if (line_read)
    {
        free(line_read);
        line_read = (char *)NULL;
    }

    /* Get a line from the user. */
    line_read = readline(prompt);

    /* If the line has any text in it,
       save it on the history. */
    if (line_read && *line_read)
        add_history(line_read);

    return (line_read);
}
