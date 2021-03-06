#include <avr/interrupt.h>
#include <string.h>
#include <math.h>
#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_Z 3

const byte interruptPin = 19;//button

volatile unsigned char cube[8][8];
volatile int current_layer = 0;

volatile int top_layer = 0;
volatile int top_layer_x;

volatile int current_width = 2;
volatile int prev_width = 2;
volatile int current_speed = 1500;
volatile int current_x;

volatile int stationary[8][6];

volatile bool gameStatus = true; // true = running, false = game over
volatile bool gameOver; // false = lose , true = win
volatile bool initialDemo = true;
volatile bool stopBouncing = false;

volatile long last_interrupt_time = 0;

void setup()
{ int i;
//Pins 22 bis 50 als Ausgänge festsetzen
for(i=22; i<50; i++) pinMode(i, OUTPUT);

// pinMode(A0, OUTPUT) as specified in the arduino reference didn't work. So I accessed the registers directly.
DDRC = 0xff;
PORTC = 0x00;

// Reset any PWM configuration that the arduino may have set up automagically!
TCCR2A = 0x00;
TCCR2B = 0x00;

// zähler, Uhr initialisieren
TCCR2A |= (0x01 << WGM21); // CTC mode. clear counter on TCNT2 == OCR2A
OCR2A = 10; // Interrupt every 25600th cpu cycle (256*100)
TCNT2 = 0x00; // start counting at 0
TCCR2B |= (0x01 << CS22) | (0x01 << CS21); // Start the clock with a 256 prescaler
TIMSK2 |= (0x01 << OCIE2A); //Timer Interrupt Mask Register

//set pin mode of interrupt pin
pinMode(interruptPin,INPUT_PULLUP); 
//attach button interrupt
attachInterrupt(digitalPinToInterrupt(interruptPin), buttonPressed, FALLING);
}

//interrupt
ISR (TIMER2_COMPA_vect)
{
int i;
// PORTA = 8x Databus (8-Bit-Latches) PIN 22-30 (bei uno Pins 0-7 PortD)
// PORTL = 3x Adressbus + OE
// PORTC = 8x Ebenen (Layer)
// Char cube [ 8 ] enthält 64 Bits von Daten für die Halteanordnung
// all layer selects off
PORTC = 0x00;
PORTL &= 0x0f; // PortL 3xAdressbus
PORTL |= 0x08; // output enable off.

// Zählen bis 8
for (i=0; i<8; i++)
{
PORTA = cube[current_layer][i]; //(PortA = 8xDatabus) PIN 22-30 (bei uno Pins 0-7 PortD)
PORTL = (PORTL & 0xF8) | (0x07 & (i+1)); // (i+1) => 74HC138 erhält die folgende Sequenz: 1 2 3 4 5 6 7 0 (muss immer eins voraus sein)
}
PORTL &= 0b00110111; // Output enable on.

//ebenen
if (current_layer < 8)
{
PORTC = (0x01 << current_layer);
}
current_layer++;
if (current_layer == 8)
current_layer = 0;
}


void loop()
{
  int i,x,y,z;
  
  while (true)
  {
    
    // effect_planboing(AXIS_Z, 400);
    // effect_planboing(AXIS_Y, 400);
    // effect_planboing(AXIS_X, 400);
    
//     effect_blinky2();
    
    // effect_random_filler(75,1);
//     effect_random_filler(75,0);
    
    // effect_rain(100);
   
    // effect_boxside_randsend_parallel (AXIS_X, 0, 150, 1);
    // effect_boxside_randsend_parallel (AXIS_X, 1, 150, 1);
    // effect_boxside_randsend_parallel (AXIS_Y, 0, 150, 1);
    // effect_boxside_randsend_parallel (AXIS_Y, 1, 150, 1);
    // effect_boxside_randsend_parallel (AXIS_Z, 0, 150, 1);
    // effect_boxside_randsend_parallel (AXIS_Z, 1, 150, 1);

//     one_by_one_turn_on_leds(1);
//     i_drew_a_green_box();
    // can_i_move_the_box(5);
//     turn_on_layers(1);
    //effect_stringfly2("1111");
    if(initialDemo == true){
      for(int i=0;i<3;i++){
        effect_loadbar(2000);  
      }
      initialDemo = false;  
    }
    
    if (gameStatus)
    {
      if (top_layer < 8) {
        bounce_stacker(top_layer, current_width, current_speed);
      }
      else
      {
        fill(0x00);
      }
      
      //capture();
    }
    else
    {
      if (gameOver) {
        delay_ms(15000);
        effect_random_filler(75,1);
        fireworks (4, 4, 400);
      }
      else {
        box_falldown();
      }
    }
    
   

  }
}


// ==========================================================================================
//   Effect functions
// ==========================================================================================

void draw_positions_axis (char axis, unsigned char positions[64], int invert)
{
  int x, y, p;
  
  fill(0x00);
  
  for (x=0; x<8; x++)
  {
    for (y=0; y<8; y++)
    {
      if (invert)
      {
        p = (7-positions[(x*8)+y]);
      } else
      {
        p = positions[(x*8)+y];
      }
    
      if (axis == AXIS_Z)
        setvoxel(x,y,p);
        
      if (axis == AXIS_Y)
        setvoxel(x,p,y);
        
      if (axis == AXIS_X)
        setvoxel(p,y,x);
    }
  }
  
}


void effect_boxside_randsend_parallel (char axis, int origin, int delay, int mode)
{
  int i;
  int done;
  unsigned char cubepos[64];
  unsigned char pos[64];
  int notdone = 1;
  int notdone2 = 1;
  int sent = 0;
  
  for (i=0;i<64;i++)
  {
    pos[i] = 0;
  }
  
  while (notdone)
  {
    if (mode == 1)
    {
      notdone2 = 1;
      while (notdone2 && sent<64)
      {
        i = rand()%64;
        if (pos[i] == 0)
        {
          sent++;
          pos[i] += 1;
          notdone2 = 0;
        }
      }
    } else if (mode == 2)
    {
      if (sent<64)
      {
        pos[sent] += 1;
        sent++;
      }
    }
    
    done = 0;
    for (i=0;i<64;i++)
    {
      if (pos[i] > 0 && pos[i] <7)
      {
        pos[i] += 1;
      }
        
      if (pos[i] == 7)
        done++;
    }
    
    if (done == 64)
      notdone = 0;
    
    for (i=0;i<64;i++)
    {
      if (origin == 0)
      {
        cubepos[i] = pos[i];
      } else
      {
        cubepos[i] = (7-pos[i]);
      }
    }
    
    
    delay_ms(delay);
    draw_positions_axis(axis,cubepos,0);

  }
  
}

void one_by_one_turn_on_leds (int iterations)
{
  for (int i=0; i<iterations; i++)
  {
    for (int z=0; z<8; z++)
    {
      for (int x=0; x<8; x++)
      {
        for (int y=0; y<8; y++)
        {
          delay_ms(100);
          setvoxel(x,y,z);
        }
      } 
    }
    fill(0x00);
  }
}

void i_drew_a_green_box ()
{
  box_filled(0, 0, 0, 0, 7, 7);
  delay_ms(3000);
  fill(0x00);
}

void can_i_move_the_box (int iterations) {
  
  for (int n=0; n<iterations; n++) {
    box_filled(0, 0, 0, 0, 7, 7);
    for (int i=0; i<8; i++)
    {
      delay_ms(1000);
      shift(AXIS_X,1);    
    }

  }

  fill(0x00);
}

void turn_on_layers(int iterations) {
  for (int i=0; i<8; i++) {
    setplane_z(i);
    delay_ms(10000);
    clrplane_z(i);
  }
}

void bounce_stacker(int layer, int current_width, int speed)
{ 
  stopBouncing = false;
  if (top_layer <8) {
    for (int i=0;i<7-current_width;i++)
    {
      if (gameStatus && !stopBouncing) {
        noInterrupts();
        fill(0x00);
        draw_stationary();
        box_filled(i, 0, layer, i+current_width, 7, (layer+1));
        current_x = i;
        interrupts();
        delay_ms(speed);
      }
    }
    
    for (int i=7-current_width;i>=0;i--)
    {
      if (gameStatus && !stopBouncing) {
        noInterrupts();
        fill(0x00);
        draw_stationary();
        box_filled(i, 0, layer, i+current_width, 7, (layer+1));
        current_x = i;
        interrupts();
        delay_ms(speed);
      }
    }
  }
}

void buttonPressed(){
   unsigned long interrupt_time = millis();
   // If interrupts come faster than 200ms, assume it's a bounce and ignore
   if (interrupt_time - last_interrupt_time > 240) 
   {
      last_interrupt_time = interrupt_time;
      capture();
   }
   
    
}

void draw_stationary()
{
    for (int i=0; i<(((top_layer))/2); i++)
    {
      box_filled(stationary[i][0], stationary[i][1], stationary[i][2], stationary[i][3], stationary[i][4], stationary[i][5]);
    }
  
}

void reset_game ()
{
  fill(0x00);
  for (int i=0; i<8; i++)
  {
    for (int j=0; j<6; j++)
    {
      stationary[i][j] = 0;
    }
  }
  prev_width =  2;
  current_width = 2;
  current_speed = 1500;
  top_layer = 0 ;
  gameStatus = true;
}

void capture()
{
  if (gameStatus == false)
    reset_game();
  else
  {    
    stopBouncing = true;
    
    // check to see if it lines up with the previous layers
    bool linesUp = false;
    if (top_layer == 0)
      linesUp = true;
    if ( (current_x >= top_layer_x) && (current_x <= (top_layer_x + prev_width)) )
      linesUp = true;
    if ( ((current_x+current_width) >= top_layer_x) && ((current_x+current_width) <= (top_layer_x + prev_width)) )
      linesUp = true;

    if (linesUp) {
      int index = top_layer/2;
      stationary[index][0] = current_x;
      stationary[index][1] = 0;
      stationary[index][2] = top_layer;
      stationary[index][3] = current_x + current_width;
      stationary[index][4] = 7;
      stationary[index][5] = top_layer+1;
      top_layer = top_layer + 2;
      top_layer_x = current_x;
      current_speed = current_speed - 300;

      if (current_width > 1)
      {
        prev_width = current_width;
        current_width = current_width - 1;
      }

      if (top_layer == 6)
      {
        prev_width = current_width;
        current_width = 0;
      }
      
      if (top_layer >= 8) {
        gameStatus = false;
        gameOver = true;
      }
    }
    else {
      gameStatus = false;
      gameOver = false;
  //    box_falldown();
    }
  }
}

void box_falldown ()
{
  for (int i=top_layer; i>=0; i--)
  {
    fill(0x00);
    draw_stationary();
    box_filled(current_x, 0, i, (current_x + current_width), 7, (i + 1));
    delay_ms(10000);
  }
  
}

void effect_rain (int iterations)
{
  int i, ii;
  int rnd_x;
  int rnd_y;
  int rnd_num;
  
  for (ii=0;ii<iterations;ii++)
  {
    rnd_num = rand()%4;
    
    for (i=0; i < rnd_num;i++)
    {
      rnd_x = rand()%8;
      rnd_y = rand()%8;
      setvoxel(rnd_x,rnd_y,7);
    }
    
    delay_ms(1000);
    shift(AXIS_Z,-1);
  }
}

// Set or clear exactly 512 voxels in a random order.
void effect_random_filler (int delay, int state)
{
  int x,y,z;
  int loop = 0;
  
  
  if (state == 1)
  {
    fill(0x00);
  } else
  {
    fill(0xff);
  }
  
  while (loop<511)
  {
    x = rand()%8;
    y = rand()%8;
    z = rand()%8;

    if ((state == 0 && getvoxel(x,y,z) == 0x01) || (state == 1 && getvoxel(x,y,z) == 0x00))
    {
      altervoxel(x,y,z,state);
      delay_ms(delay);
      loop++;
    } 
  }
}


void effect_blinky2()
{
  int i,r;
  fill(0x00);
  
  for (r=0;r<2;r++)
  {
    i = 750;
    while (i>0)
    {
      fill(0x00);
      delay_ms(i);
      
      fill(0xff);
      delay_ms(100);
      
      i = i - (15+(1000/(i/10)));
    }
    
    delay_ms(1000);
    
    i = 750;
    while (i>0)
    {
      fill(0x00);
      delay_ms(751-i);
      
      fill(0xff);
      delay_ms(100);
      
      i = i - (15+(1000/(i/10)));
    }
  }

}

// Draw a plane on one axis and send it back and forth once.
void effect_planboing (int plane, int speed)
{
  int i;
  for (i=0;i<8;i++)
  {
    fill(0x00);
        setplane(plane, i);
    delay_ms(speed);
  }
  
  for (i=7;i>=0;i--)
  {
    fill(0x00);
        setplane(plane,i);
    delay_ms(speed);
  }
}

void sendvoxel_z (unsigned char x, unsigned char y, unsigned char z, int delay)
{
  int i, ii;
  for (i=0; i<8; i++)
  {
    if (z == 7)
    {
      ii = 7-i;
      clrvoxel(x,y,ii+1);
    } else
    {
      ii = i;
      clrvoxel(x,y,ii-1);
    }
    setvoxel(x,y,ii);
    delay_ms(delay);
  }
}



// Light all leds layer by layer,
// then unset layer by layer
void effect_loadbar(int delay)
{
  fill(0x00);
  
  int z,y;
  
  for (z=0;z<8;z++)
  {
    setplane_z(z);
    delay_ms(delay);
  }
  
  for (z=7;z>=0;z--)
  {
    clrplane_z(z);
    delay_ms(delay);
  }
}

void fireworks (int iterations, int n, int delay)
{
  
  fill(0x00);

  int i,f,e;

  float origin_x = 3;
  float origin_y = 3;
  float origin_z = 3;

  int rand_y, rand_x, rand_z;

  float slowrate, gravity;

  // Particles and their position, x,y,z and their movement, dx, dy, dz
  float particles[n][6];

  for (i=0; i<iterations; i++)
  {
    if (gameStatus == false) {

    origin_x = rand()%4;
    origin_y = rand()%4;
    origin_z = rand()%2;
    origin_z +=5;
        origin_x +=2;
        origin_y +=2;

    // shoot a particle up in the air
    for (e=0;e<origin_z;e++)
    {
      setvoxel(origin_x,origin_y,e);
      delay_ms(600+500*e);
      fill(0x00);
    }

    // Fill particle array
    for (f=0; f<n; f++)
    {
      // Position
      particles[f][0] = origin_x;
      particles[f][1] = origin_y;
      particles[f][2] = origin_z;
      
      rand_x = rand()%200;
      rand_y = rand()%200;
      rand_z = rand()%200;

      // Movement
      particles[f][3] = 1-(float)rand_x/100; // dx
      particles[f][4] = 1-(float)rand_y/100; // dy
      particles[f][5] = 1-(float)rand_z/100; // dz
    }

    // explode
    for (e=0; e<25; e++)
    {
      slowrate = 1+tan((e+0.1)/20)*10;
      
      gravity = tan((e+0.1)/20)/2;

      for (f=0; f<n; f++)
      {
        particles[f][0] += particles[f][3]/slowrate;
        particles[f][1] += particles[f][4]/slowrate;
        particles[f][2] += particles[f][5]/slowrate;
        particles[f][2] -= gravity;

        setvoxel(particles[f][0],particles[f][1],particles[f][2]);


      }

      delay_ms(delay);
      fill(0x00);
    }
    }

  }

}

// ==========================================================================================
//   Draw functions
// ==========================================================================================


// Set a single voxel to ON
void setvoxel(int x, int y, int z)
{
  if (inrange(x,y,z))
    cube[z][y] |= (1 << x);
}


// Set a single voxel to ON
void clrvoxel(int x, int y, int z)
{
  if (inrange(x,y,z))
    cube[z][y] &= ~(1 << x);
}



// This function validates that we are drawing inside the cube.
unsigned char inrange(int x, int y, int z)
{
  if (x >= 0 && x < 8 && y >= 0 && y < 8 && z >= 0 && z < 8)
  {
    return 0x01;
  } else
  {
    // One of the coordinates was outside the cube.
    return 0x00;
  }
}

// Get the current status of a voxel
unsigned char getvoxel(int x, int y, int z)
{
  if (inrange(x,y,z))
  {
    if (cube[z][y] & (1 << x))
    {
      return 0x01;
    } else
    {
      return 0x00;
    }
  } else
  {
    return 0x00;
  }
}

// In some effect we want to just take bool and write it to a voxel
// this function calls the apropriate voxel manipulation function.
void altervoxel(int x, int y, int z, int state)
{
  if (state == 1)
  {
    setvoxel(x,y,z);
  } else
  {
    clrvoxel(x,y,z);
  }
}

// Flip the state of a voxel.
// If the voxel is 1, its turned into a 0, and vice versa.
void flpvoxel(int x, int y, int z)
{
  if (inrange(x, y, z))
    cube[z][y] ^= (1 << x);
}

// Makes sure x1 is alwas smaller than x2
// This is usefull for functions that uses for loops,
// to avoid infinite loops
void argorder(int ix1, int ix2, int *ox1, int *ox2)
{
  if (ix1>ix2)
  {
    int tmp;
    tmp = ix1;
    ix1= ix2;
    ix2 = tmp;
  }
  *ox1 = ix1;
  *ox2 = ix2;
}

// Sets all voxels along a X/Y plane at a given point
// on axis Z
void setplane_z (int z)
{
  int i;
  if (z>=0 && z<8)
  {
    for (i=0;i<8;i++)
      cube[z][i] = 0xff;
  }
}

// Clears voxels in the same manner as above
void clrplane_z (int z)
{
  int i;
  if (z>=0 && z<8)
  {
    for (i=0;i<8;i++)
      cube[z][i] = 0x00;
  }
}

void setplane_x (int x)
{
  int z;
  int y;
  if (x>=0 && x<8)
  {
    for (z=0;z<8;z++)
    {
      for (y=0;y<8;y++)
      {
        cube[z][y] |= (1 << x);
      }
    }
  }
}

void clrplane_x (int x)
{
  int z;
  int y;
  if (x>=0 && x<8)
  {
    for (z=0;z<8;z++)
    {
      for (y=0;y<8;y++)
      {
        cube[z][y] &= ~(1 << x);
      }
    }
  }
}

void setplane_y (int y)
{
  int z;
  if (y>=0 && y<8)
  {
    for (z=0;z<8;z++)
      cube[z][y] = 0xff;
  } 
}

void clrplane_y (int y)
{
  int z;
  if (y>=0 && y<8)
  {
    for (z=0;z<8;z++)
      cube[z][y] = 0x00; 
  }
}

void setplane (char axis, unsigned char i)
{
    switch (axis)
    {
        case AXIS_X:
            setplane_x(i);
            break;
        
       case AXIS_Y:
            setplane_y(i);
            break;

       case AXIS_Z:
            setplane_z(i);
            break;
    }
}

void clrplane (char axis, unsigned char i)
{
    switch (axis)
    {
        case AXIS_X:
            clrplane_x(i);
            break;
        
       case AXIS_Y:
            clrplane_y(i);
            break;

       case AXIS_Z:
            clrplane_z(i);
            break;
    }
}

// Fill a value into all 64 byts of the cube buffer
// Mostly used for clearing. fill(0x00)
// or setting all on. fill(0xff)
void fill (unsigned char pattern)
{
  int z;
  int y;
  for (z=0;z<8;z++)
  {
    for (y=0;y<8;y++)
    {
      cube[z][y] = pattern;
    }
  }
}



// Draw a box with all walls drawn and all voxels inside set
void box_filled(int x1, int y1, int z1, int x2, int y2, int z2)
{
  int iy;
  int iz;

  argorder(x1, x2, &x1, &x2);
  argorder(y1, y2, &y1, &y2);
  argorder(z1, z2, &z1, &z2);

  for (iz=z1;iz<=z2;iz++)
  {
    for (iy=y1;iy<=y2;iy++)
    {
      cube[iz][iy] |= byteline(x1,x2);
    }
  }

}

// Darw a hollow box with side walls.
void box_walls(int x1, int y1, int z1, int x2, int y2, int z2)
{
  int iy;
  int iz;
  
  argorder(x1, x2, &x1, &x2);
  argorder(y1, y2, &y1, &y2);
  argorder(z1, z2, &z1, &z2);

  for (iz=z1;iz<=z2;iz++)
  {
    for (iy=y1;iy<=y2;iy++)
    { 
      if (iy == y1 || iy == y2 || iz == z1 || iz == z2)
      {
        cube[iz][iy] = byteline(x1,x2);
      } else
      {
        cube[iz][iy] |= ((0x01 << x1) | (0x01 << x2));
      }
    }
  }

}

// Draw a wireframe box. This only draws the corners and edges,
// no walls.
void box_wireframe(int x1, int y1, int z1, int x2, int y2, int z2)
{
  int iy;
  int iz;

  argorder(x1, x2, &x1, &x2);
  argorder(y1, y2, &y1, &y2);
  argorder(z1, z2, &z1, &z2);

  // Lines along X axis
  cube[z1][y1] = byteline(x1,x2);
  cube[z1][y2] = byteline(x1,x2);
  cube[z2][y1] = byteline(x1,x2);
  cube[z2][y2] = byteline(x1,x2);

  // Lines along Y axis
  for (iy=y1;iy<=y2;iy++)
  {
    setvoxel(x1,iy,z1);
    setvoxel(x1,iy,z2);
    setvoxel(x2,iy,z1);
    setvoxel(x2,iy,z2);
  }

  // Lines along Z axis
  for (iz=z1;iz<=z2;iz++)
  {
    setvoxel(x1,y1,iz);
    setvoxel(x1,y2,iz);
    setvoxel(x2,y1,iz);
    setvoxel(x2,y2,iz);
  }

}

// Returns a byte with a row of 1's drawn in it.
// byteline(2,5) gives 0b00111100
char byteline (int start, int end)
{
  return ((0xff<<start) & ~(0xff<<(end+1)));
}

// Flips a byte 180 degrees.
// MSB becomes LSB, LSB becomes MSB.
char flipbyte (char byte)
{
  char flop = 0x00;

  flop = (flop & 0b11111110) | (0b00000001 & (byte >> 7));
  flop = (flop & 0b11111101) | (0b00000010 & (byte >> 5));
  flop = (flop & 0b11111011) | (0b00000100 & (byte >> 3));
  flop = (flop & 0b11110111) | (0b00001000 & (byte >> 1));
  flop = (flop & 0b11101111) | (0b00010000 & (byte << 1));
  flop = (flop & 0b11011111) | (0b00100000 & (byte << 3));
  flop = (flop & 0b10111111) | (0b01000000 & (byte << 5));
  flop = (flop & 0b01111111) | (0b10000000 & (byte << 7));
  return flop;
}

// Draw a line between any coordinates in 3d space.
// Uses integer values for input, so dont expect smooth animations.
void line(int x1, int y1, int z1, int x2, int y2, int z2)
{
  float xy; // how many voxels do we move on the y axis for each step on the x axis
  float xz; // how many voxels do we move on the y axis for each step on the x axis 
  unsigned char x,y,z;
  unsigned char lasty,lastz;

  // We always want to draw the line from x=0 to x=7.
  // If x1 is bigget than x2, we need to flip all the values.
  if (x1>x2)
  {
    int tmp;
    tmp = x2; x2 = x1; x1 = tmp;
    tmp = y2; y2 = y1; y1 = tmp;
    tmp = z2; z2 = z1; z1 = tmp;
  }

  
  if (y1>y2)
  {
    xy = (float)(y1-y2)/(float)(x2-x1);
    lasty = y2;
  } else
  {
    xy = (float)(y2-y1)/(float)(x2-x1);
    lasty = y1;
  }

  if (z1>z2)
  {
    xz = (float)(z1-z2)/(float)(x2-x1);
    lastz = z2;
  } else
  {
    xz = (float)(z2-z1)/(float)(x2-x1);
    lastz = z1;
  }



  // For each step of x, y increments by:
  for (x = x1; x<=x2;x++)
  {
    y = (xy*(x-x1))+y1;
    z = (xz*(x-x1))+z1;
    setvoxel(x,y,z);
  }
  
}

// Delay loop.
// This is not calibrated to milliseconds,
// but we had allready made to many effects using this
// calibration when we figured it might be a good idea
// to calibrate it.
void delay_ms(uint16_t x)
{
  uint8_t y, z;
  for ( ; x > 0 ; x--){
    for ( y = 0 ; y < 90 ; y++){
      for ( z = 0 ; z < 6 ; z++){
        asm volatile ("nop");
      }
    }
  }
}



// Shift the entire contents of the cube along an axis
// This is great for effects where you want to draw something
// on one side of the cube and have it flow towards the other
// side. Like rain flowing down the Z axiz.
void shift (char axis, int direction)
{
  int i, x ,y;
  int ii, iii;
  int state;

  for (i = 0; i < 8; i++)
  {
    if (direction == -1)
    {
      ii = i;
    } else
    {
      ii = (7-i);
    } 
  
  
    for (x = 0; x < 8; x++)
    {
      for (y = 0; y < 8; y++)
      {
        if (direction == -1)
        {
          iii = ii+1;
        } else
        {
          iii = ii-1;
        }
        
        if (axis == AXIS_Z)
        {
          state = getvoxel(x,y,iii);
          altervoxel(x,y,ii,state);
        }
        
        if (axis == AXIS_Y)
        {
          state = getvoxel(x,iii,y);
          altervoxel(x,ii,y,state);
        }
        
        if (axis == AXIS_X)
        {
          state = getvoxel(iii,y,x);
          altervoxel(ii,y,x,state);
        }
      }
    }
  }
  
  if (direction == -1)
  {
    i = 7;
  } else
  {
    i = 0;
  } 
  
  for (x = 0; x < 8; x++)
  {
    for (y = 0; y < 8; y++)
    {
      if (axis == AXIS_Z)
        clrvoxel(x,y,i);
        
      if (axis == AXIS_Y)
        clrvoxel(x,i,y);
      
      if (axis == AXIS_X)
        clrvoxel(i,y,x);
    }
  }
}


