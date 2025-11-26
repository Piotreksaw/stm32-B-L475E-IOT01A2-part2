Co udalo sie zrobic MP:
1) przeslac .json ktory wychodzi po UART poprzez ble*,
2) zrobic skrypt w Python, ktory zbiera dane z uart i wyswietla je na wykresach oddzielnych dla kazdego czujnika

Wnioski i pomysly:
1) .json z danymi z czujnikow jest wysylany w kilku ramkach notify, ktore/dlatego ze maja maksymalny payload 20 charow,
byc moze mozna to zrobic czyms innym niz notify i wyslac wszystko w jednej ramce (nie zdazylem sprawdzic)
2) w pythonowym skrypcie mozna dodac/zmodyfikowac funkcjonalnosc tak zeby wysylal dane na thingsboard

*Prosze kogos z Androidem o sprawdzenie czy on sie zgadza z tym co idzie po UART, bo jedyna aplikacja na ktorej
udalo mi sie obejrzec payload tej ramki to nRF connect, a na zgnilymJablku nie dekoduje payloadu z HEXa i idzie oszalec