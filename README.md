# LSH Shell – Lekka powłoka systemowa w C

LSH to minimalistyczna powłoka systemowa napisana w języku C, obsługująca:

- potoki (`|`)
- przekierowania wejścia/wyjścia/błędów (`<`, `>`, `2>`)
- uruchamianie procesów w tle (`&`)
- zakończenie przez `exit`
- sygnały jak `Ctrl+C`, `Ctrl+Z`

---

## Funkcjonalność

| Funkcja             | Opis                                                               |
|---------------------|--------------------------------------------------------------------|
| `cd [folder]`       | Zmiana katalogu roboczego                                          |
| `exit`              | Zakończenie działania powłoki                                      |
| `komenda &`         | Uruchomienie komendy w tle                                         |
| `komenda < plik`    | Przekierowanie wejścia z pliku                                     |
| `komenda > plik`    | Przekierowanie wyjścia do pliku                                    |
| `komenda 2> plik`   | Przekierowanie błędów do pliku                                     |
| `komenda1 | komenda2` | Potok, przekazanie wyniku jednej komendy do drugiej             |
| Obsługa sygnałów    | `SIGINT` (Ctrl+C), `SIGTSTP` (Ctrl+Z), `SIGCHLD` (procesy w tle)  |

---

## Instalacja

1. **Sklonuj repozytorium lub skopiuj pliki źródłowe.**

2. **Skompiluj program za pomocą GCC:**

```bash
gcc -o lsh shell.c
```

3. **Uruchom powłokę:**

```bash
./lsh
```

## Wymagania
1. **Kompilator GCC**
2. **System Unix/Linux (powłoka korzysta z POSIX API)**