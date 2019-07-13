
/*
 * Lightweight ACPI Implementation
 * Copyright (C) 2018-2019 the lai authors
 */

// Internal header file. Do not use outside of LAI.

#pragma once

#include <lai/core.h>

struct lai_amlname {
    int is_absolute;   // Is the path absolute or not?
    int height;        // Number of scopes to exit before resolving the name.
                       // In other words, this is the number of ^ in front of the name.
    int search_scopes; // Is the name searched in the scopes of all parents?

    // Internal variables used by the parser.
    const uint8_t *it;
    const uint8_t *end;
};

// Initializes the AML name parser.
// Use lai_amlname_done() + lai_amlname_iterate() to process the name.
size_t lai_amlname_parse(struct lai_amlname *amln, const void *data);

// Returns true if there are no more segments.
int lai_amlname_done(struct lai_amlname *amln);

// Copies the next segment of the name to out.
// out must be a char array of size >= 4.
void lai_amlname_iterate(struct lai_amlname *amln, char *out);

// Turns the AML name into a ASL-like name string.
// Returns a pointer allocated by laihost_malloc().
char *lai_stringify_amlname(const struct lai_amlname *amln);

// This will replace lai_resolve().
lai_nsnode_t *lai_do_resolve(lai_nsnode_t *ctx_handle, const struct lai_amlname *amln);

// Used in the implementation of lai_resolve_new_node().
void lai_do_resolve_new_node(lai_nsnode_t *node,
        lai_nsnode_t *ctx_handle, const struct lai_amlname *amln);

// Evaluate constant data (and keep result).
//     Primitive objects are parsed.
//     Names are left unresolved.
//     Operations (e.g. Add()) are not allowed.
#define LAI_DATA_MODE 1
// Evaluate dynamic data (and keep result).
//     Primitive objects are parsed.
//     Names are resolved. Methods are executed.
//     Operations are allowed and executed.
#define LAI_OBJECT_MODE 2
// Like LAI_OBJECT_MODE, but discard the result.
#define LAI_EXEC_MODE 3
#define LAI_REFERENCE_MODE 4
#define LAI_IMMEDIATE_WORD_MODE 5

// Allocate a new package.
int lai_create_string(lai_variable_t *, size_t);
int lai_create_c_string(lai_variable_t *, const char *);
int lai_create_buffer(lai_variable_t *, size_t);
int lai_create_pkg(lai_variable_t *, size_t);

void lai_load(lai_state_t *, struct lai_operand *, lai_variable_t *);
void lai_store(lai_state_t *, struct lai_operand *, lai_variable_t *);

void lai_exec_get_objectref(lai_state_t *, struct lai_operand *, lai_variable_t *);
void lai_exec_get_integer(lai_state_t *, struct lai_operand *, lai_variable_t *);

int lai_exec_method(lai_nsnode_t *, lai_state_t *, int, lai_variable_t *);

// --------------------------------------------------------------------------------------
// Inline function for execution stack manipulation.
// --------------------------------------------------------------------------------------

// Pushes a new item to the execution stack and returns it.
static inline lai_stackitem_t *lai_exec_push_stack_or_die(lai_state_t *state) {
    state->stack_ptr++;
    if (state->stack_ptr == 16)
        lai_panic("execution engine stack overflow");
    return &state->stack[state->stack_ptr];
}

// Returns the n-th item from the top of the stack.
static inline lai_stackitem_t *lai_exec_peek_stack(lai_state_t *state, int n) {
    if (state->stack_ptr - n < 0)
        return NULL;
    return &state->stack[state->stack_ptr - n];
}

// Returns the last item of the stack.
static inline lai_stackitem_t *lai_exec_peek_stack_back(lai_state_t *state) {
    return lai_exec_peek_stack(state, 0);
}

// Returns the lai_stackitem_t pointed to by the state's context_ptr.
static inline lai_stackitem_t *lai_exec_peek_stack_at(lai_state_t *state, int n) {
    if (n < 0)
        return NULL;
    return &state->stack[n];
}

// Removes n items from the stack.
static inline void lai_exec_pop_stack(lai_state_t *state, int n) {
    state->stack_ptr -= n;
}

// Removes the last item from the stack.
static inline void lai_exec_pop_stack_back(lai_state_t *state) {
    lai_exec_pop_stack(state, 1);
}

// --------------------------------------------------------------------------------------
// Inline function for opstack manipulation.
// --------------------------------------------------------------------------------------

// Pushes a new item to the opstack and returns it.
static inline struct lai_operand *lai_exec_push_opstack_or_die(lai_state_t *state) {
    if (state->opstack_ptr == 16)
        lai_panic("operand stack overflow");
    struct lai_operand *object = &state->opstack[state->opstack_ptr];
    memset(object, 0, sizeof(struct lai_operand));
    state->opstack_ptr++;
    return object;
}

// Returns the n-th item from the opstack.
static inline struct lai_operand *lai_exec_get_opstack(lai_state_t *state, int n) {
    if (n >= state->opstack_ptr)
        lai_panic("opstack access out of bounds"); // This is an internal execution error.
    return &state->opstack[n];
}

// Removes n items from the opstack.
static inline void lai_exec_pop_opstack(lai_state_t *state, int n) {
    for (int k = 0; k < n; k++) {
        struct lai_operand *operand = &state->opstack[state->opstack_ptr - k - 1];
        if (operand->tag == LAI_OPERAND_OBJECT)
            lai_var_finalize(&operand->object);
    }
    state->opstack_ptr -= n;
}
