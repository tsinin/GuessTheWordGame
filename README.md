Реализованы сервер и клиент для игры в "Угадай слово". Игра заключается в том, что сервер загадывает рандомное слово из базы слов и отправляет его клиенту. Далее клиент должен вводить по одной букве, пытаясь угадать буквы из слова. Если он угадывает, в слове появляются найденные буквы. Задача клиента - за (дефолтно, можно поменять вторым параметром серверу) 12 шагов полностью открыть слово (сделать без звёздочек). Если клиент хочет закончить, начать новое слово или же узнать текущее и начать новое - должен написать соответствующие символы '#', '!', '?'.

Сами файлы компилируются через gcc. 
Запускать сервер из его папки с 1м аргументом в виде порта, и со 2м (опционально) с ограничением на кол-во попыток для букв одного слова. Также при запуске сервера, файл с базой слов должен лежать рядом.

Запускать клиент с 2мя аргументами (первый всегда 127.0.0.1, костыль, буквально не успеваю исправить), второй - порт, на котором прослушивает сервер.
