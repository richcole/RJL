{ sys |

  sys PcxFontParser: {

    read_header: {
      self stream read_chars: 4;
    };

    read_int32 {
      self stream read_int32;
    };

    read_lsb_int32 {
      self stream read_int32;
    };

    init: {
      PCF_PROPERTIES:         0x1;
      PCF_ACCELERATORS:       0x2;
      PCF_METRICS:            0x4;
      PCF_BITMAPS:            0x8;
      PCF_INK_METRICS:        0x10;
      PCF_BDF_ENCODING:       0x20;
      PCF_SWIDTHS:            0x40;
      PCF_GLYPH_NAMES:        0x80;
      PCF_BDF_ACCELERATOR:    0x100;
      PCF_DEFAULT_FORMAT:     0x00000000;
      PCF_INKBOUNDS:          0x00000200;
      PCF_ACCEL_W_INKBOUNDS:  0x00000100;
      PCF_COMPRESSED_METRICS: 0x00000100;
      PCF_GLYPH_PAD_MASK: 0x3;
      PCF_BYTE_MASK:      0x4;
      PCF_BIT_MASK:       0x8;
      PCF_SCAN_UNIT_MASK: 0x30;
    };

    read_from:: { stream |
      self stream;
      self header: read_header;
      self table_count: read_lsb_int32;
      self tocs: (
        ( 0 .. table_count ) map: { index |
          read_toc;
        }
      );
      self tables: (
        self tocs map: { toc |
          read_table: toc;
        };
      );
    };
    
    read_toc: { 
      (|
        self type: read_lsb_int32;
        self format: read_lsb_int32;
        self size: read_lsb_int32;
        self offset: read_lsb_int32;
      |)
    };

    read_table:: { toc |
      if ( toc == PCF_PROPERTIES ) {
        read_prop toc;
      }
      elsif ( toc == PCF_ACCELERATORS ) {
        read_accel toc;
      };
    };

    read_prop: {
      (|
        self format read_lsb_int32;
        self nprops read_int32;
        self props  (( 0 .. nprops ) map { index |
          (|
            self offset         read_int32;
            self is_string_prop read_int8;
            self value          read_int32;
          |);
        });
        pad32;
        self string_size read_int32;
        self string      read_bytes[string_size];
        pad32;
        at:: { index |
          prop: (self props at: index)
          key: self string substring: (prop offset):
          if ( prop is_string_prop ) {
            self string substring: (prop value):
          }
          else {
            prop_value;
          };
        };
      |)
    };

Metrics Data

Several of the tables (PCF_METRICS, PCF_INK_METRICS, and within the accelerator tables) contain metrics data which may be in either compressed (PCF_COMPRESSED_METRICS) or uncompressed (DEFAULT) formats. The compressed format uses bytes to contain values, while the uncompressed uses shorts. The (compressed) bytes are unsigned bytes which are offset by 0x80 (so the actual value will be (getc(pcf_file)-0x80). The data are stored as:
Compressed

uint8 left_sided_bearing;
uint8 right_side_bearing;
uint8 character_width;
uint8 character_ascent;
uint8 character_descent;
 /* Implied character attributes field = 0 */
Uncompressed

int16 left_sided_bearing;
int16 right_side_bearing;
int16 character_width;
int16 character_ascent;
int16 character_descent;
uint16 character_attributes;
This provides the data needed for an XCharStruct.

Accelerator Tables

These data provide various bits of information about the font as a whole. This data structure is used by two tables PCF_ACCELERATORS and PCF_BDF_ACCELERATORS. The tables may either be in DEFAULT format or in PCF_ACCEL_W_INKBOUNDS (in which case they will have some extra metrics data at the end.

The accelerator tables look like:

lsbint32 format; 		/* Always stored with least significant byte first! */
uint8 noOverlap;		/* if for all i, max(metrics[i].rightSideBearing - metrics[i].characterWidth) */
				/* 	<= minbounds.leftSideBearing */
uint8 constantMetrics;		/* Means the perchar field of the XFontStruct can be NULL */
uint8 terminalFont;		/* constantMetrics true and forall characters: */
				/* 	the left side bearing==0 */
				/*	the right side bearing== the character's width */
				/*	the character's ascent==the font's ascent */
				/*	the character's descent==the font's descent */
uint8 constantWidth;		/* monospace font like courier */
uint8 inkInside;		/* Means that all inked bits are within the rectangle with x between [0,charwidth] */
				/*  and y between [-descent,ascent]. So no ink overlaps another char when drawing */
uint8 inkMetrics;		/* true if the ink metrics differ from the metrics somewhere */
uint8 drawDirection;		/* 0=>left to right, 1=>right to left */
uint8 padding;
int32 fontAscent;		/* byte order as specified in format */
int32 fontDescent;
int32 maxOverlap;		/* ??? */
Uncompressed_Metrics minbounds;
Uncompressed_Metrics maxbounds;
/* If format is PCF_ACCEL_W_INKBOUNDS then include the following fields */
    Uncompressed_Metrics ink_minbounds;
    Uncompressed_Metrics ink_maxbounds;
/* Otherwise those fields are not in the file and should be filled by duplicating min/maxbounds above */
BDF Accelerators should be preferred to plain Accelerators if both tables are present. BDF Accelerators contain data that refers only to the encoded characters in the font (while the simple Accelerator table includes all glyphs), therefore the BDF Accelerators are more accurate.

Metrics Tables

There are two different metrics tables, PCF_METRICS and PCF_INK_METRICS, the former contains the size of the stored bitmaps, while the latter contains the minimum bounding box. The two may contain the same data, but many CJK fonts pad the bitmaps so all bitmaps are the same size. The table format may be either DEFAULT or PCF_COMPRESSED_METRICS (see the section on Metrics Data for an explanation).

lsbint32 format; 		/* Always stored with least significant byte first! */
/* if the format is compressed */
    int16 metrics_count;
    Compressed_Metrics metrics[metrics_count];
/* else if format is default (uncompressed) */
    int32 metrics_count;
    Uncompressed_Metrics metrics[metrics_count];
/* endif */
The Bitmap Table

The bitmap table has type PCF_BITMAPS. Its format must be PCF_DEFAULT.

lsbint32 format; 		/* Always stored with least significant byte first! */
int32 glyph_count;		/* byte ordering depends on format, should be the same as the metrics count */
int32 offsets[glyph_count];	/* byte offsets to bitmap data */
int32 bitmapSizes[4];		/* the size the bitmap data will take up depending on various padding options */
				/*  which one is actually used in the file is given by (format&3) */
char bitmap_data[bitmapsizes[format&3]];	/* the bitmap data. format contains flags that indicate: */
				/* the byte order (format&4 => LSByte first)*/
				/* the bit order (format&8 => LSBit first) */
				/* how each row in each glyph's bitmap is padded (format&3) */
				/*  0=>bytes, 1=>shorts, 2=>ints */
				/* what the bits are stored in (bytes, shorts, ints) (format>>4)&3 */
				/*  0=>bytes, 1=>shorts, 2=>ints */

The Encoding Table

The encoding table has type PCF_BDF_ENCODINGS. Its format must be PCF_DEFAULT.

lsbint32 format; 		/* Always stored with least significant byte first! */
int16 min_char_or_byte2;	/* As in XFontStruct */
int16 max_char_or_byte2;	/* As in XFontStruct */
int16 min_byte1;		/* As in XFontStruct */
int16 max_byte1;		/* As in XFontStruct */
int16 default_char;		/* As in XFontStruct */
int16 glyphindeces[(max_char_or_byte2-min_char_or_byte2+1)*(max_byte1-min_byte1+1)];
				/* Gives the glyph index that corresponds to each encoding value */
				/* a value of 0xffff means no glyph for that encoding */
For single byte encodings min_byte1==max_byte1==0, and encoded values are between [min_char_or_byte2,max_char_or_byte2]. The glyph index corresponding to an encoding is glyphindex[encoding-min_char_or_byte2].

Otherwise [min_byte1,max_byte1] specifies the range allowed for the first (high order) byte of a two byte encoding, while [min_char_or_byte2,max_char_or_byte2] is the range of the second byte. The glyph index corresponding to a double byte encoding (enc1,enc2) is glyph_index[(enc1-min_byte1)*(max_char_or_byte2-min_char_or_byte2+1)+ enc2-min_char_or_byte2].

Not all glyphs need to be encoded. Not all encodings need to be associated with glyphs.

The Scalable Widths Table

The encoding table has type PCF_SWIDTHS. Its format must be PCF_DEFAULT.

lsbint32 format; 		/* Always stored with least significant byte first! */
int32 glyph_count;		/* byte ordering depends on format, should be the same as the metrics count */
int32 swidths[glyph_count];	/* byte offsets to bitmap data */
The scalable width of a character is the width of the corresponding postscript character in em-units (1/1000ths of an em).

The Glyph Names Table

The encoding table has type PCF_GLYPH_NAMES. Its format must be PCF_DEFAULT.

lsbint32 format; 		/* Always stored with least significant byte first! */
int32 glyph_count;		/* byte ordering depends on format, should be the same as the metrics count */
int32 offsets[glyph_count];	/* byte offsets to string data */
int32 string_size;
char string[string_size];
The postscript name associated with each character.