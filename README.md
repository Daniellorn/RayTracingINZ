# RayTracingINZ

Praca inżynierska. Projekt przedstawia algorytm ray tracingu w compute shaderze.

## Budowanie

Aby uruchomić program trzeba zainstalować Visual Studio 2022 (w tym środowisku projekt był budowany i testowany).
Wraz z IDE należy zainstalować pakiety: "Programowanie aplikacji klasycznych w języku C++" i "projektowanie gier przy użyciu języka C++". Aby zbudować projekt wymagany jest python (testowana wersja - 3.13.7). Potrzebny będzie także CMake, dostępny w zmiennych systemowych aby można było wykonać komendę zawartą w skrypcie BuildDependencies.py (testowana wersja - 4.1.1). 

Następnie

```
git clone --recursive <repo-url>
```

Uruchamiamy skrypt GenerateProject.bat. Skrypt zbuduje solucje projektu którą otwieramy w środowisku Visual Studio i bibliotekę Assimp. 

## Sterowanie

Przytrzymać prawy przycisk myszy + WSAD, aby poruszać się po scenie. Spacja - góra, lewy shift - dół.