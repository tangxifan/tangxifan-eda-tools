// MCluster mapper
// Creator : Pierre - Emmanuel Gaillardon
// General configurations of MPack

#ifndef MPACK_H_
#define MPACK_H_

// Global configuration fuses

/// Support for fixed fanin/fanout cells only 
#define GATE_MAX_INPUT_NB   2
#define GATE_MAX_OUTPUT_NB  1

#define HASHSIZE 4095
#define NAMELENGTH 16   // Length of the name stored for each net
#define BUFSIZE 300     // Maximum line length for various parsing proc.
//#define DEBUG           // Echoes input & checks error conditions
//#define VERBOSE_DEBUG   // Prints all sorts of intermediate data

// Specific definitions

// List of supported blocks and gate primitives
enum block_types {INPAD = -2, OUTPAD, GATE, BUF, LATCH, EMPTY};
enum gate_types {NONE =0, ZERO, ONE, INV1A, INV1B, BUF1A, BUF1B, AND2, NAND2, OR2, NOR2, NOTAND2, ANDNOT2, NOTOR2, ORNOT2, XOR2, XNOR2};

#define DRIVER 0     // Is a pin driving a net or in the fanout?
#define RECEIVER 1
#define OPEN -1      // Pin is unconnected

// Macro definitions
#define max(a,b) (((a) > (b))? (a) : (b))
#define min(a,b) ((a) > (b)? (b) : (a))

#ifndef TRUE // Some compilers predefine TRUE, FALSE */
typedef enum {FALSE, TRUE} boolean;
#else
typedef int boolean;
#endif

// Data structures definitions

struct hash_nets {char *name; int index; int count; 
   struct hash_nets *next;}; 
// count is the number of pins on this net so far

struct s_net {char *name; int num_pins; int *pins;};
// name:  ASCII net name for informative annotations in the output.
// num_pins:  Number of pins on this net.
// pins[]: Array containing the blocks to which the pins of this net
//         connect.  Output in pins[0], inputs in other entries.

struct s_block {char *name; enum block_types type; enum gate_types gate_type; int num_nets; int num_in; int num_out; int nets[GATE_MAX_INPUT_NB+GATE_MAX_OUTPUT_NB];};  
// name:  Taken from the net which it drives.
// type:  LUT, INPAD, OUTPAD or LATCH.
// num_nets:  number of nets connected to this block.
// nets[]:  List of nets connected to this block.  Net[0] is the
//          output, others are inputs, except for OUTPAD.  OUTPADs
//          only have an input, so this input is in net[0].

#endif
