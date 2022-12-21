// Importamos las bibliotecas
#include <Servo.h>
#include <SoftwareSerial.h>

// bluetooth
SoftwareSerial BT(10, 11);  //6 7  10 11 (rx, tx)

unsigned char *buffer;
String mensaje = "";

// Creamos los servos
Servo garra;
Servo brazoAzul;
Servo brazoRojo;
Servo base;

// Posición inicial
int pos;

// Estados de la lectura
#define LEYENDO 1
#define LEIDO 2
#define ESPERANDO 0

// Estados del robot
String State = "Init";

void setup() {
  garra.attach(3);
  base.attach(5);
  brazoAzul.attach(6);
  brazoRojo.attach(9);
  Serial.begin(9600);
  BT.begin(9600);
  buffer = malloc(1000);

  ////// Posiciones iniciales ///////////
  brazoAzul.write(30);
  brazoRojo.write(20);
  garra.write(120);
  base.write(0);

  esperarArriba();
}

//////////////// MOVER GARRA //////////////////
void cerrarGarra() {
  for (int pos = 90; pos <= 130; pos += 1) {
    garra.write(pos);
    delay(15);
  }
  // delay(500);
}

void abrirGarra() {
  for (int pos = 130; pos >= 90; pos -= 1) {
    garra.write(pos);
    delay(15);
  }
  // delay(500);
}

//////////////// MOVER AMBOS BRAZOS //////////////////
void moverRojoAzulArriba() {
  for (pos = 50; pos >= 0; pos -= 1) {
    int azul = 130 - pos;
    int rojo = 20 + pos;
    brazoAzul.write(azul);
    brazoRojo.write(rojo);
    delay(50);
  }
}

void moverRojoAzulArribaMitad() {
  for (pos = 50; pos >= 0; pos -= 1) {
    int azul = 175 - pos;
    brazoAzul.write(azul);
    delay(50);
  }
}

void moverRojoAzulAbajo() {
  for (pos = 0; pos <= 50; pos += 1) {
    int azul = 130 - pos;
    int rojo = 20 + pos;
    brazoAzul.write(azul);
    brazoRojo.write(rojo);
    delay(50);
  }
}


//////////////// MOVER BASE //////////////////
void moverBase() {
  for (pos = 70; pos <= 140; pos += 1) {
    base.write(pos);
    delay(50);
  }
}

void moverBase2() {
  for (pos = 140; pos >= 70; pos -= 1) {
    base.write(pos);
    delay(50);
  }
}

////////////// STAND BY ///////////////
void esperarArriba() {
  brazoRojo.write(20);
  brazoAzul.write(60);
  garra.write(120);
  delay(2000);
}

///////// Mover desde arriba hasta una altura media ///////////
void salirArriba() {
  for (pos = 170; pos >= 125; pos -= 1) {
    brazoAzul.write(pos);
    delay(50);
  }
}

///////// Comunicación, código del Andrés ////////////////

int st_read = ESPERANDO;
int size = 0;

void recibir() {
  unsigned char c;
  if (BT.available()) {

    //Serial.write(BT.read());
    c = BT.read();
    // Serial.println(char(c));
    if (st_read == LEYENDO && c == '}') {
      size = size + 1;
      buffer[size] = c;
      for (int i = 0; i < size + 1; i++) {
        mensaje = mensaje + String(char(buffer[i]));
      }
      st_read = LEIDO;
    }
    if (st_read == ESPERANDO && c == '{') {
      size = 0;
      st_read = LEYENDO;
      buffer[size] = c;
    }
    if (st_read == LEYENDO && c != '{' && c != '}') {
      size = size + 1;
      buffer[size] = c;
    }
  }
}

String recibirMensaje() {

  // Recibimos el mensaje
  recibir();

  if (mensaje_disp()) {
    String aux = leerMensaje();
    return aux;
  }
}

void movimiento() {
  moverBase();
  abrirGarra();
  moverRojoAzulAbajo();
  cerrarGarra();
  moverRojoAzulArriba();
  moverBase2();
  salirArriba();
  abrirGarra();
  moverRojoAzulArribaMitad();
  esperarArriba();
}

int mensaje_disp() {
  if (st_read == LEIDO) {
    return 1;
  }
  return 0;
}

String leerMensaje() {
  String aux = mensaje;
  mensaje = "";
  st_read = ESPERANDO;
  return aux;
}

// Hay que activar las funciones en el loop para que esto se mueva
void loop() {
  String mensaje = "";
  mensaje = recibirMensaje();

  if (State == "Init" && mensaje[1] == 's') {
    State = "Ready";
  }

  if (State == "Ready" && mensaje[3] == 'c') {
    State = "Moving";

    Serial.println("Cargando...");
    movimiento();
    BT.write('a');
    Serial.println("Carga terminada...");

    State = "Ready";
  }

  if (State == "Ready" && mensaje[3] == 'e' || State == "Init" && mensaje[3] == 'e') {
    State = "Init";
  }
}