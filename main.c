#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
typedef struct s_cmd
{
    char **cmds;
    int type;
    int pre;
} t_cmd;

typedef struct s_vars
{
    t_cmd *cmd;
    int pipe[2];
    int num_of_command;
} t_vars;

int ft_strlen(char *str)
{
    int i = 0;
    while(str[i])
        i++;
    return (i);
}
char *ft_strdup(char *str)
{
    int i = 0;
    char *new;
    new = malloc(sizeof(char ) * (ft_strlen(str) + 1));
    while(str[i])
    {
        new[i] = str[i];
        i++;
    }
    new[i] = '\0';
    return (new);
}

void   add_command(t_vars *vars, int index, int i, int j, char **av, int type)
{

    vars->cmd[index].cmds = malloc(sizeof(char *) * (i - j + 1));
    int tmp = 0;
    while(j < i)
    {
        vars->cmd[index].cmds[tmp] = ft_strdup(av[j]);
        tmp++;
        j++;
    }
    vars->cmd[index].cmds[tmp] = NULL;
    vars->cmd[index].type = type;
    if(index == 0)
        vars->cmd[index].pre = 0;
    else
        vars->cmd[index].pre = vars->cmd[index - 1].type;
}

void cd_error1()
{
    write(2, "error: cd: bad arguments\n", ft_strlen("error: cd: bad arguments\n"));
    exit(1);
}


void cd_error2(char *str)
{
    write(2, "error: cd: cannot change directory to ", ft_strlen("error: cd: cannot change directory to "));
    write(2, str, ft_strlen(str));
    write(1,"\n", 1);
    exit(1);
}
void is_cd(char **Path)
{
    int size = 0;
    while(Path[size])
        size++;
    if(size != 2)
        cd_error1();
    else if (chdir(Path[1]) != 0)
        cd_error2(Path[1]);
    
}

void execeve_error(char *str)
{
    write(2, "error: cannot execute executable_that_failed\n", ft_strlen("error: cannot execute executable_that_failed\n"));
    write(2, str, ft_strlen(str));
    write(1, "\n", 1);
}

void start_exec(t_vars *vars, char **env)
{
    int i =0;
    int pid;
    int p;
    while(i < vars->num_of_command)
    {
        if(!strcmp(vars->cmd[i].cmds[0], "cd"))
            is_cd(vars->cmd[i].cmds);
        else
        {
            if(vars->cmd[i].type == 1)
                pipe(vars->pipe);
            pid = fork();
            if(!pid)
            {
                if(vars->cmd[i].type == 1)
                    dup2(vars->pipe[1],1);
                if(vars->cmd[i].pre == 1)
                    dup2(p, 0);
                if(execve(vars->cmd[i].cmds[0], vars->cmd[i].cmds, env) == -1)
                    execeve_error(vars->cmd[i].cmds[0]);
                exit(1);
            }
            if(vars->cmd[i].type == 2 || vars->cmd[i].pre == 2)
                waitpid(pid, NULL, 0);
            if(vars->cmd[i].type == 1)
            {
                close(vars->pipe[1]);
                p = dup(vars->pipe[0]);
            }
            if(vars->cmd[i].pre == 1)
                close(vars->pipe[0]);
        }
        i++;
    }
    close(p);
    i = vars->num_of_command;
    while(i--)
    {
        if(vars->cmd[i].type == 1 || vars->cmd[i].pre == 1)
            waitpid(-1, NULL,0);
    }
}

int main(int ac, char **av, char **env)
{
    av++;
    t_vars vars;
    (void)ac;
    vars.num_of_command = 1;
    int  i = 0;
    while(av[i])
    {
        if(!strcmp(av[i], ";") || !strcmp(av[i], "|"))
            vars.num_of_command++;
        i++;
    }
    vars.cmd = malloc(sizeof(t_cmd) * (vars.num_of_command + 1));
    i = 0;
    int index = 0;
    int type  = 0;
    int j = 0;
    while(av[i])
    {
        type = 0;
        if(!strcmp(av[i], ";") || !strcmp(av[i], "|"))
        {
            if(!strcmp(av[i], "|"))
                type = 1;
            else
                type = 2;
            add_command(&vars, index, i, j, av, type);
            index++;
            j = i + 1;
        }
        i++;
    }
    add_command(&vars, index, i, j, av, type);
    start_exec(&vars, env);
}