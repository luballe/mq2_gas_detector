# Por qué el sensor MQ-2 presenta lecturas erráticas al inicio

El comportamiento errático inicial del MQ-2 es **completamente normal** y se debe a procesos fisicoquímicos bien documentados en el material semiconductor SnO₂. Las lecturas comienzan altas y decrecen monotónicamente durante 24-48 horas hasta alcanzar el equilibrio termodinámico. Este patrón predecible—descenso exponencial hacia una línea base estable—es la firma distintiva que diferencia el comportamiento intrínseco del sensor de problemas de conexión o del microcontrolador.

## La física del SnO₂ explica el comportamiento inicial

El MQ-2 utiliza dióxido de estaño (SnO₂), un **semiconductor tipo-n** cuya resistencia depende de la ionosorción de oxígeno en su superficie. El mecanismo opera en tres fases temperatura-dependientes:

| Temperatura | Especie dominante | Reactividad |
|-------------|-------------------|-------------|
| <150°C | O₂⁻ (superóxido) | Baja—forma "sin salida" |
| 150-300°C | O⁻ (oxígeno atómico) | Alta—zona de operación |
| >300°C | O²⁻ (oxígeno reticular) | Muy alta |

Durante el calentamiento, el sensor atraviesa secuencialmente estas fases. El oxígeno molecular se adsorbe en la superficie y captura electrones de la banda de conducción: O₂(ads) + e⁻ → O₂⁻ → O⁻ + e⁻. Este proceso forma una **capa de agotamiento electrónico** que establece barreras de potencial tipo Schottky en los límites de grano, incrementando la resistencia base en aire limpio.

La inestabilidad inicial resulta de cinco fenómenos simultáneos: gradientes térmicos antes del equilibrio, transición entre especies de oxígeno ionosorbido, desorción de contaminantes acumulados (agua, hidrocarburos, VOCs), competencia dinámica por sitios de adsorción, y formación progresiva de la capa de agotamiento. **La resistencia decrece exponencialmente** siguiendo R(t) ≈ R_base + (R_inicial - R_base) × e^(-t/τ), donde τ típicamente es 10-30 minutos para estabilización gruesa.

## Especificaciones técnicas del datasheet sobre precalentamiento

Los datasheets de Winsen y Hanwei establecen requisitos específicos que varían significativamente según el historial del sensor:

| Escenario | Tiempo de precalentamiento |
|-----------|---------------------------|
| Sensor nuevo o almacenado >1 mes | **24-48 horas** (burn-in inicial) |
| Almacenamiento 1-4 semanas | 20-30 minutos |
| Uso reciente (días) | **5-10 minutos** |

El elemento calefactor—una bobina de aleación Ni-Cr enrollada en un tubo cerámico de Al₂O₃—presenta estas características: voltaje de 5.0V±0.1V, resistencia de **30-33Ω±5%** a temperatura ambiente, y consumo de **800-950mW**. Esta potencia eleva el SnO₂ a 200-300°C en segundos, pero la **estabilización química** requiere horas. El datasheet de Winsen especifica explícitamente que tras almacenamiento prolongado "the surface of the sensor absorbs moisture, mixed gas, pollution matters" requiriendo envejecimiento y recalibración.

## Cómo distinguir las tres causas de lecturas erráticas

La diferenciación requiere analizar la **firma característica** de cada problema:

**Warm-up normal del sensor:** Patrón monotónico descendente. Valores ADC típicos: 700-900 al encender → 400-500 a 5 min → 150-250 a 30 min → 80-150 estabilizado. Desviación estándar <5 tras estabilización, rango <20 entre muestras consecutivas.

**Conexión flotante/desconectada:** Valores aleatorios en todo el rango ADC (0-1023 en Arduino, 0-4095 en ESP32). Distribución plana en histograma, sin tendencia. Caso documentado: "CH4: 4095 - CH4: 0 - CH4: 4095" en ciclos consecutivos. **Causa frecuente:** falta de tierra común entre fuentes de alimentación separadas.

**Conexión intermitente (soldadura fría/cable suelto):** Saltos súbitos bidireccionales hacia los rieles (0V o 5V), sensible al movimiento. Patrón bimodal en histograma—valores normales con picos en los extremos. La prueba de golpeteo ("tap test") dispara cambios inmediatos.

## Diagnóstico práctico con código y osciloscopio

Para un diagnóstico sistemático, implemente esta rutina de análisis estadístico:

```cpp
void diagnoseSignal() {
  int readings[100]; long sum = 0;
  int minVal = 1023, maxVal = 0;
  
  for(int i = 0; i < 100; i++) {
    readings[i] = analogRead(MQ2PIN);
    sum += readings[i];
    minVal = min(minVal, readings[i]);
    maxVal = max(maxVal, readings[i]);
    delay(10);
  }
  
  float mean = sum / 100.0;
  float variance = 0;
  for(int i = 0; i < 100; i++) 
    variance += sq(readings[i] - mean);
  float stdDev = sqrt(variance / 100);
  int range = maxVal - minVal;
  
  // Interpretación diagnóstica
  if(range > 900) 
    Serial.println("FLOTANTE: Verificar conexiones");
  else if(range > 200 && (minVal < 50 || maxVal > 970)) 
    Serial.println("INTERMITENTE: Inspeccionar soldaduras");
  else if(stdDev > 20 && mean > 400) 
    Serial.println("WARM-UP: Esperar estabilización");
  else if(stdDev < 5) 
    Serial.println("ESTABLE: Sensor listo");
}
```

| Métrica | Estable | Flotante | Intermitente | Warm-up |
|---------|---------|----------|--------------|---------|
| StdDev | <5 | >200 | Variable | 20-100 |
| Rango | <20 | >900 | 200-1023 | Decreciente |
| Min/Max | Centrados | En rieles | Uno en riel | Alto, bajando |

Con osciloscopio, configure modo de persistencia para acumular trazas: una señal estable muestra trazo compacto, mientras conexiones sueltas dispersan trazas por toda la pantalla. El acoplamiento AC revela <50mVpp en sensor estable versus escala completa en entrada flotante.

## Pruebas físicas definitivas

**Test de golpeteo:** Golpee suavemente el módulo, breadboard y conexiones monitoreando el serial. Sin cambio = normal; picos súbitos = conexión suelta.

**Test de resistencia de tierra:** Mida resistencia entre GND del sensor y GND del Arduino. Debe ser <1Ω. Valores mayores o fluctuantes indican conexión defectuosa. Este test detectó el problema documentado donde lecturas 0-4095 se resolvieron conectando GND de la fuente externa al GND del ESP32.

**Inspección visual:** Soldaduras frías aparecen opacas, granulares o irregulares versus el brillo cóncavo de uniones correctas.

## Conclusión

El MQ-2 presenta lecturas erráticas iniciales por diseño—el proceso de ionosorción de oxígeno y formación de la capa de agotamiento electrónico requiere tiempo para alcanzar equilibrio termodinámico. La firma diagnóstica clave es el **descenso monotónico exponencial**: si sus lecturas bajan consistentemente durante minutos/horas, el sensor funciona correctamente. Valores aleatorios de rango completo indican conexión flotante (verificar tierra común); saltos súbitos sensibles al movimiento señalan soldadura fría. Para aplicaciones críticas, mantenga alimentación continua al sensor—reiniciar el ciclo de warm-up tras cada corte de energía compromete la precisión durante el período de estabilización.