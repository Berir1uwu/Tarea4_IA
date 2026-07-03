#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>
#include <fstream> 
#include <cstdlib> // Necesario para rand() y srand()
#include <ctime>   // Necesario para time()

// Estructura para representar un punto de control de Voronoi (Semilla de Bioma)
struct VoronoiSeed {
    double x, z;
    int biomeType; // 0: Desierto, 1: Bosque
};

// Función de Ruido 2D con desfase aleatorio para variar las alturas en cada ejecución
double pseudoPerlin2D(double x, double z, double frequency, double offsetX, double offsetZ) {
    double value = std::sin((x + offsetX) * frequency) * std::cos((z + offsetZ) * frequency);
    return (value + 1.0) / 2.0; // Normalizado entre 0 y 1
}

// Función para obtener el bioma más cercano usando Distancia Euclidiana (Voronoi)
int getVoronoiBiome(double x, double z, const std::vector<VoronoiSeed>& seeds) {
    double minDistance = 999999.0;
    int closestBiome = 0;
    for (const auto& seed : seeds) {
        double dist = std::sqrt(std::pow(x - seed.x, 2) + std::pow(z - seed.z, 2));
        if (dist < minDistance) {
            minDistance = dist;
            closestBiome = seed.biomeType;
        }
    }
    return closestBiome;
}

int main() {
    // Inicializar la semilla de aleatoriedad basada en el tiempo real del sistema
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Generar posiciones aleatorias para las semillas dentro del chunk de 16x16
    // Aseguramos que una semilla sea para Desierto (0) y otra para Bosque (1)
    std::vector<VoronoiSeed> biomes = {
        { static_cast<double>(std::rand() % 16), static_cast<double>(std::rand() % 16), 0 },
        { static_cast<double>(std::rand() % 16), static_cast<double>(std::rand() % 16), 1 }
    };

    // Desfases aleatorios para la función de ruido (para que las montañas cambien de lugar)
    double offsetX = std::rand() % 100;
    double offsetZ = std::rand() % 100;

    const int CHUNK_SIZE = 16;
    const int MAX_HEIGHT = 32;
    const int TEST_ITERATIONS = 10; 
    
    double totalDurationMs = 0.0;
    int finalBlocksCalculated = 0;

    // Matriz auxiliar para guardar las alturas y biomas de la última iteración y mostrarlos ordenados
    int heightMap[CHUNK_SIZE][CHUNK_SIZE];
    int biomeMap[CHUNK_SIZE][CHUNK_SIZE];

    std::cout << "--- EJECUTANDO PRUEBAS DE RENDIMIENTO (" << TEST_ITERATIONS << " ITERACIONES) ---" << std::endl;

    for (int run = 0; run < TEST_ITERATIONS; ++run) {
        int totalBlocksGenerated = 0;
        auto start = std::chrono::high_resolution_clock::now();

        // Bucle de generación del terreno
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                int biome = getVoronoiBiome(x, z, biomes);
                double noiseVal = pseudoPerlin2D(x, z, 0.2, offsetX, offsetZ);
                int height = 0;

                if (biome == 0) {
                    height = static_cast<int>(5 + noiseVal * 5);  // Desierto
                } else {
                    height = static_cast<int>(10 + noiseVal * 15); // Bosque
                }

                totalBlocksGenerated += height;

                // Guardamos los datos de la última corrida para los mapas visuales
                if (run == TEST_ITERATIONS - 1) {
                    heightMap[x][z] = height;
                    biomeMap[x][z] = biome;
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        totalDurationMs += duration.count();
        finalBlocksCalculated = totalBlocksGenerated; 
    }

    // --- IMPRESIÓN DE MAPAS ALEATORIOS EN CONSOLA ---

    std::cout << "   1. VISTA TOPOGRAFICA DEL MAPA ALEATORIO" << std::endl;
    std::cout << "Leyenda: [~] Valle Desierto  [=] Duna Desierto\n"
              << "         [_] Valle Bosque    [^] Montana Bosque\n" << std::endl;

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            if (biomeMap[x][z] == 0) {
                if (heightMap[x][z] > 8) std::cout << "= ";
                else std::cout << "~ ";
            } else {
                if (heightMap[x][z] > 14) std::cout << "^ ";
                else std::cout << "_ ";
            }
        }
        std::cout << std::endl;
    }


    std::cout << "   2. MATRIZ DE ALTURAS VERTICALES (EJEY)" << std::endl;
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            // Formatear la salida para que los números de un dígito no desalineen la matriz
            if (heightMap[x][z] < 10) std::cout << "0" << heightMap[x][z] << " ";
            else std::cout << heightMap[x][z] << " ";
        }
        std::cout << std::endl;
    }

    double averageDurationMs = totalDurationMs / TEST_ITERATIONS;

    std::cout << "\nESTADÍSTICAS" << std::endl;
    std::cout << "Semillas Voronoi generadas en: (" << biomes[0].x << "," << biomes[0].z << ") y (" << biomes[1].x << "," << biomes[1].z << ")" << std::endl;
    std::cout << "Bloques sólidos calculados: " << finalBlocksCalculated << std::endl;
    std::cout << "Tiempo de procesamiento promedio: " << averageDurationMs << " ms" << std::endl;

    // --- EXPORTACIÓN AL ARCHIVO DE TEXTO ---
    std::ofstream txtFile("metricas_procedimental.txt");
    if (txtFile.is_open()) {
        txtFile << "   METRICAS DE GENERACION PROCEDIMENTAL (CHUNK)   \n";
        txtFile << "Dimensiones evaluadas      : " << CHUNK_SIZE << " x " << CHUNK_SIZE << " voxeles\n";
        txtFile << "Limite vertical maximo     : " << MAX_HEIGHT << " niveles\n";
        txtFile << "Muestra total de bloques   : " << finalBlocksCalculated << " bloques solidos\n";
        txtFile << "Iteraciones de control     : " << TEST_ITERATIONS << " corridas de CPU\n";
        txtFile << "Tiempo promedio por chunk  : " << averageDurationMs << " ms\n";
        txtFile.close();
    }

    return 0;
}