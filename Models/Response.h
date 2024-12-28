#pragma once

enum class Response
{
    OK, // проверка выполнена
    BACK, // шаг назад
    REPLAY, // переигровка
    QUIT, // выход
    CELL // нажитие на клетку
};
