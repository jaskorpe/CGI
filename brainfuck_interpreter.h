int interpret (int ignore);

int register_print_func (void (*cb) (char));

int init_interpreter (int code_len, char *code,
                      int input_len, char *input,
                      void (*cb) (char));
