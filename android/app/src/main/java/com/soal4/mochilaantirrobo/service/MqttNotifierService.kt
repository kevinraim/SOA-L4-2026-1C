package com.soal4.mochilaantirrobo.service

import android.util.Log
import org.eclipse.paho.client.mqttv3.MqttClient
import org.eclipse.paho.client.mqttv3.MqttConnectOptions
import org.eclipse.paho.client.mqttv3.MqttException
import org.eclipse.paho.client.mqttv3.MqttMessage
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine

object MqttNotifierService {

    private lateinit var mqttClient: MqttClient

    private const val BROKER_URL = "tcp://192.168.30.39:1883"
    private const val CLIENT_ID = "MochilaAndroidClient"

    private const val TOPIC_SHAKE = "my/shake"
    private const val TOPIC_SENSIBILIDAD = "my/topic"

    fun conectar() {

            try {

                mqttClient = MqttClient(BROKER_URL, CLIENT_ID, MemoryPersistence())

                val options = MqttConnectOptions().apply {
                    isAutomaticReconnect = true
                    isCleanSession = false
                    userName = "claromio"
                    password = "RiverBest".toCharArray()
                }

                mqttClient.connect(options)

                if (!mqttClient.isConnected) {
                    println("Error al conectarse por mqtt")
                }


            } catch (e: Exception) {
                e.printStackTrace()
            }
        }

    suspend fun enviarShake(): Void =
        suspendCoroutine {
            val topic = TOPIC_SHAKE

            val message = MqttMessage("true".toByteArray()).apply {
                qos = 1
            }

            try {

                mqttClient.publish(topic, message)

            } catch (e: MqttException) {
                Log.e("MQTT", "Error notificando shake por MQTT")
                e.printStackTrace()
            }
        }

    suspend fun enviarSensibilidad(nivel: Int): String =
        suspendCoroutine { continuation ->

            val topic = TOPIC_SENSIBILIDAD

            val payload = nivel.toString()

            val message = MqttMessage(payload.toByteArray()).apply {
                qos = 1
            }

            try {

                mqttClient.publish(topic, message)

                continuation.resume(
                    "MQTT: Nivel $nivel enviado con éxito"
                )

            } catch (e: MqttException) {

                continuation.resume(
                    "MQTT Error: ${e.message}"
                )
            }
        }
}