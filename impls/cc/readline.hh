#pragma once

#include "env.hh"

void initialize_readline(const Env *env);

/* Read a string, and return a pointer to it.
   Returns NULL on EOF. */
char *rl_gets(const char *prompt);
