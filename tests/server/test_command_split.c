#include <string.h>

#include "compass.h"
#include "data_structure/event.h"

Test(command_split, test_simple_command)
{
    char *cmd[COMMAND_WORD_COUNT] = { nullptr };
    char buff[] = "Look";
    size_t count = 0;

    assert("Should return true since it is a valid command",
        command_split(buff, cmd, strlen(buff)) == true);
    for (count = 0; cmd[count] != nullptr; count++)
        ;
    assert("Has one arg", count == 1);
    assert("Equals \"Look\"", !strcmp(cmd[0], "Look"));
}

Test(command_split, test_quoted_args)
{
    char *cmd[COMMAND_WORD_COUNT] = { nullptr };
    char buff[] = "Broadcast \"Hello penguin\"";
    size_t count = 0;

    assert("Should return true since it is a valid command",
        command_split(buff, cmd, strlen(buff)) == true);
    for (count = 0; cmd[count] != nullptr; count++)
        ;
    assert("There is only 2 arguments", count == 2);
    assert("command", !strcmp(cmd[0], "Broadcast"));
    assert("message", !strcmp(cmd[1], "Hello penguin"));
}

Test(command_split, ending_with_newline)
{
    char *cmd[COMMAND_WORD_COUNT] = { nullptr };
    char buff[] = "Broadcast \"Hello penguin\"\n";
    size_t count = 0;

    assert("Should return true since it is a valid command",
        command_split(buff, cmd, strlen(buff)) == true);
    for (count = 0; cmd[count] != nullptr; count++)
        ;
    assert("There is only 2 arguments", count == 2);
    assert("command", !strcmp(cmd[0], "Broadcast"));
    assert("message", !strcmp(cmd[1], "Hello penguin"));
}

Test(command_split, test_empty_command)
{
    char *cmd[COMMAND_WORD_COUNT] = { nullptr };
    char buff[] = "";
    size_t count = 0;

    assert("Should return true since it is a valid command",
        command_split(buff, cmd, strlen(buff)) == true);
    for (count = 0; cmd[count] != nullptr; count++)
        ;
    assert("There is no arguments", count == 0);
}

Test(command_split, test_command_with_spaces)
{
    char *cmd[COMMAND_WORD_COUNT] = { nullptr };
    char buff[] = "   Look   ";
    size_t count = 0;

    assert("Should return true since it is a valid command",
        command_split(buff, cmd, strlen(buff)) == true);
    for (count = 0; cmd[count] != nullptr; count++)
        ;
    assert("Has one arg", count == 1);
    assert("Equals \"Look\"", !strcmp(cmd[0], "Look"));
}

Test(command_split, invalid_quote)
{
    char *cmd[COMMAND_WORD_COUNT] = { nullptr };
    char buff[] = "Broadcast \"Hello penguin";

    assert("Should return false because of invalid quote",
        command_split(buff, cmd, strlen(buff)) == false);
}

Test(command_split, more_than_max_args)
{
    char *cmd[COMMAND_WORD_COUNT] = { nullptr };
    char buff[] = "Look at the sky and see the stars";

    assert("Should return false because of too many arguments",
        command_split(buff, cmd, strlen(buff)) == false);
}
