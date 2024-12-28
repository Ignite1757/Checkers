#pragma once

enum class Response
{
    OK, // check completed
    BACK, // go back
    REPLAY, // replay game
    QUIT, // exit the program
    CELL // clicking on a cell
};
