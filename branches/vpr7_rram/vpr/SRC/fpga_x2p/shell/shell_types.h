typedef struct s_shell_env t_shell_env;
struct s_shell_env {
  t_arch arch;
  t_vpr_setup vpr_setup;
};

typedef struct s_shell_cmd t_shell_cmd;
struct s_shell_cmd {
    char* name;
    t_opt_info* opts;
    void (*execute)(t_shell_env*, t_opt_info*);
};

#define LAST_CMD_NAME "EOC"

