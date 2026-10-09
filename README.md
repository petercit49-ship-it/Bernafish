# Bernafish 1.0

**Autor:** MF Pedro Bernabé Moreno Maura  
**Basado en:** Stockfish 4 (por Tord Romstad, Marco Costalba y Joona Kiiski)  
**Licencia:** GPLv3 (ver `Copying.txt`)  
**Estado:** Versión estable 1.0

---

## ¿Qué es Bernafish?

**Bernafish** es un motor de ajedrez humanizado derivado de Stockfish 4. Su objetivo no es ser el motor más fuerte, sino el más **configurable**: permite crear personalidades de juego completas (estilo, Elo objetivo, errores humanos, agresividad, preferencias posicionales, etc.) mediante un archivo INI externo.

Está diseñado para ser usado desde:
- **Ajedrez Evolutivo**, una plataforma de entrenamiento integral en Python.
- **Cualquier GUI UCI** (ChessBase, Arena, CuteChess, Fritz, BanksiaGUI, etc.).
- **Modo consola**, sin GUI, para automatización.

---

## Características principales

### Sistema de personalidades (INI)

Bernafish lee personalidades desde un archivo INI (por defecto `personalities.ini`). Cada personalidad es una sección con más de 50 parámetros:

```ini
[Personalidad: Kasparov]
descripcion = ELO 2750 — Táctico. Dinamismo e iniciativa.
etiquetas_estilo = dinámico, ataque, iniciativa
elo_objetivo = 2750
limite_fuerza_motor = 2100
limite_profundidad = 99
valor_peon_mg = 100
valor_peon_eg = 100
...
ataque_rey = 200
tormenta_peones = 180
...
escala_inexactitudes = 10
escala_errores = 0

Parámetros configurables (extracto)
Grupo	Ejemplos
Valores de piezas (MG/EG)	valor_peon_mg, valor_dama_eg, etc.
Posicionales	estructura_peones, cadena_peones, outpost, valor_espacio
Tácticos	ataque_rey, tormenta_peones, bonus_presion_piezas
Intercambio	contempt_dinamico, bonus_par_alfiles, torre_septima_fila
Movilidad y seguridad	movilidad, seguridad_rey
Preferencias de piezas	preferencia_dama, preferencia_torre, etc.
Estilo dinámico	optimismo_dinamico, factor_especulacion
Humanización	escala_inexactitudes, escala_errores, aprendizaje_activado
Fuerza	elo_objetivo, limite_fuerza_motor, limite_profundidad

Humanización
Imprecisiones controladas: el motor puede cometer errores pequeños de forma intencional.

Errores graves ajustables: simula el comportamiento de jugadores humanos de distintos niveles.

Escala de errores: de 0 (juego perfecto) a 100 (muy humano).

Profundidad limitada: simula jugadores que no calculan hasta el final.

Sistema de experiencia
Libro de aperturas personal por personalidad.

Aprendizaje adaptativo (opcional).

Estadísticas por partida (victorias, tablas, derrotas).

Formato .exp compatible con la plataforma Ajedrez Evolutivo.

Elo objetivo
Cada personalidad puede apuntar a un Elo concreto (de 900 a 2900). El motor ajusta automáticamente la profundidad y los errores para acercarse a ese nivel.

¿Qué versión descargar?
Consulta la sección Releases del repositorio. Se publican binarios para distintas CPUs.

CPU	Recomendación
Intel 2013+ / AMD 2015+	bernafish-x86-64-modern
Muy antigua o desconocida	bernafish-x86-64
Instalación
Descarga el binario desde la sección Releases.

Colócalo en la carpeta que prefieras.

(Opcional) Coloca el archivo personalities.ini en la misma carpeta.

Ábrelo desde tu GUI de ajedrez como motor UCI.

Requisito en Windows: el archivo libwinpthread-1.dll debe estar junto al .exe.

Uso
Desde una GUI UCI (ChessBase, Arena, etc.)
Añade el motor como UCI.

Configura las opciones que necesites (Elo, personalidad, etc.).

Juega o analiza.

Desde la consola
bash
./bernafish
uci
setoption name Skill Level value 10
position startpos
go depth 15
quit
Desde Ajedrez Evolutivo (Python)
La plataforma carga el INI y traduce las claves al motor. Ver la documentación de Ajedrez Evolutivo para más detalles.

Compilación
bash
make build ARCH=x86-64-modern COMP=mingw
Para builds optimizadas con PGO:

bash
make profile-build ARCH=x86-64-modern COMP=mingw
Consulta make help para ver todas las opciones.

Opciones UCI principales
Bernafish expone más de 90 opciones UCI. Las principales:

Opción	Tipo	Descripción
Book File	string	Archivo de libro PolyGlot
OwnBook	check	Usar libro propio
Skill Level	spin 0-20	Fuerza general del motor
Elo	spin 800-2900	Elo objetivo (humanización)
Imprecision	spin 0-100	Nivel de imprecisiones humanas
Blunder Rate	spin 0-100	Tasa de errores graves
Depth Limit	spin 0-40	Profundidad máxima
Aggressiveness	spin 0-200	Agresividad general
Cowardice	spin 0-200	Tendencia defensiva
Experience File	string	Archivo .exp de aprendizaje
Experience Learning	check	Activar aprendizaje
Experience Use	spin 0-100	Porcentaje de uso del libro
Para la lista completa: envía uci al motor y revisa la salida.

Estructura del proyecto
text
Bernafish/
├── Copying.txt              # Licencia GPLv3
├── Readme.md                # Este archivo
├── CHANGELOG.md             # Historial de cambios
├── Makefile                 # Sistema de compilación
├── personalities.ini        # Personalidades por defecto (ejemplo)
├── src/                     # Código fuente
│   ├── *.cpp, *.h           # Núcleo del motor
│   ├── material.cpp/.h      # Personalización de material (Bernafish)
│   ├── pawns.cpp/.h         # Personalización de peones (Bernafish)
│   ├── experience.cpp/.h    # Sistema de experiencia (Bernafish)
│   └── misc.cpp             # Banner UCI personalizado
└── scripts/                 # Scripts auxiliares
Créditos
Bernafish 1.0 — MF Pedro Bernabé Moreno Maura

Stockfish 4 — Tord Romstad, Marco Costalba, Joona Kiiski

Comunidad Stockfish — por mantener vivo el proyecto original

Licencia
Este proyecto se distribuye bajo la GNU General Public License v3.
Ver Copying.txt para el texto completo.

Cualquier modificación o redistribución debe mantener la misma licencia y hacer disponible el código fuente.
