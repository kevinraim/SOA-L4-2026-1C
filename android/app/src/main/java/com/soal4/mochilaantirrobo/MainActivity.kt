package com.soal4.mochilaantirrobo

import android.hardware.SensorManager
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.ui.unit.dp
import androidx.lifecycle.lifecycleScope
import androidx.navigation.NavController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.soal4.mochilaantirrobo.sensors.ShakeDetector
import com.soal4.mochilaantirrobo.service.MqttNotifierService
import com.soal4.mochilaantirrobo.ui.theme.MochilaAntirroboTheme
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch

class MainActivity : ComponentActivity() {

    private lateinit var shakeDetector: ShakeDetector

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val sensorManager = getSystemService(SENSOR_SERVICE) as SensorManager
        shakeDetector = ShakeDetector(sensorManager)
        shakeDetector.onShake = {
            lifecycleScope.launch(Dispatchers.IO) {
                MqttNotifierService.enviarShake()
            }
        }

        // TODO que no corra en una corutina
        lifecycleScope.launch(Dispatchers.IO) {
            MqttNotifierService.conectar()
        }

        enableEdgeToEdge()
        setContent {
            MochilaAntirroboTheme {
                AppNavigation()
            }
        }
    }

    // Cuando se entra en la activity
    override fun onResume() {
        super.onResume()
        shakeDetector.iniciar()
    }

    // Cuando sale de la activity
    override fun onPause() {
        super.onPause()
        shakeDetector.detener()
    }
}

// Solo dice que otros composables (pantallas) existen y cual es el principal.
@Composable
fun AppNavigation() {
    val navController = rememberNavController()
    NavHost(navController = navController, startDestination = "ajuste") {
        composable("ajuste") {
            PantallaAjuste(navController = navController)
        }
        composable("info") {
            PantallaInfo(navController = navController)
        }
    }
}

// Las pantallas acá se definen así, no con un nuevo activity.
// Se definene con @composable
@Composable
fun PantallaAjuste(navController: NavController? = null) {
    var valorSlider by remember { mutableFloatStateOf(0f) }

    // Define una corutina, corre las cosas en otro hilo que no es el principal
    // Es como crear un nuevo thread
    val scope = rememberCoroutineScope()

    val snackbarHostState = remember { SnackbarHostState() }

    Scaffold(
        snackbarHost = { SnackbarHost(snackbarHostState) }
    ) { padding ->
        Column(
            modifier = Modifier
                .padding(padding)
                .fillMaxSize(),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Text("Nivel de Sensibilidad: ${valorSlider.toInt()}")

            Slider(
                value = valorSlider,
                onValueChange = { valorSlider = it },
                valueRange = 0f..100f,
                modifier = Modifier.padding(horizontal = 40.dp)
            )

            Button(onClick = {
                // Con launch le indicas que lo corra en ese nuevo hilo, en lugar del principal.
                scope.launch {
                    println("Enviando nueva sensibilidad via MQTT")
                    val respuesta = MqttNotifierService.enviarSensibilidad(valorSlider.toInt())
                    snackbarHostState.showSnackbar(respuesta)
                }
            }) {
                Text("Enviar a la Mochila")
            }

            Spacer(modifier = Modifier.height(16.dp))

            // Con navigate le indicas que vaya a esa otra pantalla
            OutlinedButton(onClick = { navController?.navigate("info") }) {
                Text("Ir a pantalla info")
            }
        }
    }
}

// Esta es una pantalla de ejemplo para ver como navegar desde una pantalla a otra
// Después se puede borrar
@Composable
fun PantallaInfo(navController: NavController? = null) {
    Scaffold { padding ->
        Column(
            modifier = Modifier
                .padding(padding)
                .fillMaxSize(),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Text(
                text = "Proyecto SOA grupo 4",
                style = MaterialTheme.typography.headlineMedium
            )
            Spacer(modifier = Modifier.height(8.dp))
            Text("Hola! Soy un ejemplo.")
            Spacer(modifier = Modifier.height(24.dp))
            Button(onClick = { navController?.popBackStack() }) {
                Text("Volver")
            }
        }
    }
}