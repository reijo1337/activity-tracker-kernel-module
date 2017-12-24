# activity-tracker-kernel-module
Kernel module for linux which tracks user activity (X-server)

Ок. Как это работает? Для начала нам надо все собрать.

Приложение состоит из двух частей:
1. Демон. Написан на C++ с использованием Qt, собирает инфу об активности и отправляет в ядро.
1. Модуль ядра. Хранит всю инфу и дает доступ для статистики

Собирать демона надо через Qt Creator, мне было лень писать CMake =)

Собирать модуль ядра надо через команду make

Далее под root правами запускаем модуль

insmod tracker.ko

и запускаем демона

./WindowTracker

Для просмотра статистики смотрим виртуальный файл

cat /proc/time-tracker
