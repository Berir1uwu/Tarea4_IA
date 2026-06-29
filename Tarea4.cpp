#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>

// Estructura para representar un punto de control de Voronoi (Semilla de Bioma)
struct VoronoiSeed {
    double x, z;
    int biomeType; // 0: Desierto, 1: Bosque
};

// Función simple para simular Ruido de Perlin 2D (Seno/Coseno determinista)
double pseudoPerlin2D(double x, double z, double frequency) {
    double value = std::sin(x * frequency) * std::cos(z * frequency);
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
    // 1. Inicializar semillas de Voronoi para los Biomas
    std::vector<VoronoiSeed> biomes = {
        {2.0, 2.0, 0},   // Región de Desierto en la esquina inferior
        {14.0, 14.0, 1}  // Región de Bosque en la esquina superior
    };

    const int CHUNK_SIZE = 16;
    const int MAX_HEIGHT = 32;
    
    // Medir tiempo de ejecución para los datos del informe
    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "--- GENERANDO CHUNK BASE (VÓXELES 16x16) ---" << std::endl;
    
    int totalBlocksGenerated = 0;

    // 2. Bucle de generación del terreno
    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            // Calcular Bioma dominante
            int biome = getVoronoiBiome(x, z, biomes);
            
            // Ajustar altura base según el bioma usando el ruido procedural
            double noiseVal = pseudoPerlin2D(x, z, 0.2);
            int height = 0;

            if (biome == 0) {
                height = static_cast<int>(5 + noiseVal * 5); // Llanura baja (Desierto)
            } else {
                height = static_cast<int>(10 + noiseVal * 15); // Montañas (Bosque)
            }

            totalBlocksGenerated += height;

            // Imprimir una vista previa topográfica en la consola
            if (biome == 0) std::cout << ". "; // Arenas / Desierto
            else std::cout << "# ";            // Árboles / Bosque
        }
        std::cout << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;

    std::cout << "\n--- ESTADÍSTICAS DEL AVANCE ---" << std::endl;
    std::cout << "Bloques sólidos calculados: " << totalBlocksGenerated << std::endl;
    std::cout << "Tiempo de procesamiento: " << duration.count() << " ms" << std::endl;

    return 0;
}