/*
 *  TwoLAME: an optimized MPEG Audio Layer Two encoder
 *
 *  Copyright (C) 2001-2004 Michael Cheng
 *  Copyright (C) 2004-2005 The TwoLAME Project
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <twolame.h>
#include <sndfile.h>


/*
  Constants
*/
#define MP2BUFSIZE		(16384)
#define AUDIOBUFSIZE	(9210)
#define MAX_NAME_SIZE	(256)


/* 
  Global Variables
*/
int single_frame_mode = FALSE;
int downmix = FALSE;
int byteswap = FALSE;
int channelswap = FALSE;
int samplerate = 44100;
int channels = 2;

char inputfilename[MAX_NAME_SIZE] = "";
char outputfilename[MAX_NAME_SIZE] = "";



/* 
  new_extension()
  Puts a new extension name on a file name <filename>.
  Removes the last extension name, if any.
*/
static void
new_extension(char *filename, char *extname, char *newname)
{
	int             found, dotpos;

	/* First, strip the extension */
	dotpos = strlen(filename);
	found = 0;
	do {
		switch (filename[dotpos]) {
		case '.':
			found = 1;
			break;
		case '\\':
		case '/':
		case ':':
			found = -1;
			break;
		default:
			dotpos--;
			if (dotpos < 0)
				found = -1;
			break;
		}
	} while (found == 0);

	if (found == -1) {
		strcpy(newname, filename);
	}
	if (found == 1) {
		strncpy(newname, filename, dotpos);
		newname[dotpos] = '\0';
	}
	strcat(newname, extname);
}



/* 
  print_file_config() 
  Display information about input and output files
*/
static void
print_file_config()
{



}


/* 
  usage() 
  Display the extended usage information
*/
static void
usage()
{
	fprintf(stdout, "\nTwoLAME version %s (%s)\n", get_twolame_version(), get_twolame_url());
	fprintf(stdout, "MPEG Audio Layer II encoder\n\n");
	fprintf(stdout, "usage: \n");
	fprintf(stdout, "\ttwolame [options] <input> <output>\n\n");

	fprintf(stdout, "Options:\n");
	fprintf(stdout, "Input\n");
	fprintf(stdout, "\t-r          input is raw pcm\n");
	fprintf(stdout, "\t-x          force byte-swapping of input\n");
	fprintf(stdout, "\t-s sfreq    sampling frequency of raw input (kHz) (default 44.1)\n");
	fprintf(stdout, "\t-N nch      number of channels for raw input (default 2)\n");
	fprintf(stdout, "\t-g          swap channels of input file\n");
	fprintf(stdout, "\t-a          downmix from stereo to mono\n");

    //--scale <arg>   scale input (multiply PCM data) by <arg>
    //--scale-l <arg> scale channel 0 (left) input (multiply PCM data) by <arg>
    //--scale-r <arg> scale channel 1 (right) input (multiply PCM data) by <arg>

	
	fprintf(stdout, "Output\n");
	fprintf(stdout, "\t-m mode     (s)tereo, (j)oint, (m)ono or (a)uto\n");
	fprintf(stdout, "\t-p psy      psychoacoustic model 0/1/2/3 (dflt 3)\n");
	fprintf(stdout, "\t-b br       total bitrate in kbps    (dflt 192)\n");
	fprintf(stdout, "\t-v lev      vbr mode\n");
	fprintf(stdout, "\t-l lev      ATH level (dflt 0)\n");
	
	fprintf(stdout, "Operation\n");
	fprintf(stdout, "\t-q num      quick mode. only calculate psy model every num frames\n");
		
	fprintf(stdout, "Misc\n");
	fprintf(stdout, "\t-d emp      de-emphasis n/5/c        (dflt: (n)one)\n");
	fprintf(stdout, "\t-c          mark as copyright\n");
	fprintf(stdout, "\t-o          mark as original\n");
	fprintf(stdout, "\t-e          add error protection\n");
	fprintf(stdout, "\t-r          force padding bit/frame on\n");
	fprintf(stdout, "\t-D len      add DAB extensions of length [len]\n");
	fprintf(stdout, "\t-t          talkativity 0=no messages (dflt 2)\n");
	fprintf(stdout, "\t-u ind      Set the upper bitrate when in VBR mode\n");
	fprintf(stdout, "\t-R num      Set the number of reserved bits at the end of frame\n");
	fprintf(stdout, "\t-E          turn on energy level extensions\n");

	fprintf(stdout, "Files\n");
	fprintf(stdout, "\tinput       input sound file. (WAV,AIFF,PCM or use '/dev/stdin')\n");
	fprintf(stdout, "\toutput      output bit stream of encoded audio\n");
	
	fprintf(stdout, "\n\tAllowable bitrates for 16, 22.05 and 24kHz sample input\n");
	fprintf(stdout, "\t   8,  16,  24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160\n");
	fprintf(stdout, "\n\tAllowable bitrates for 32, 44.1 and 48kHz sample input\n");
	fprintf(stdout, "\t  32,  48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384\n");
	fprintf(stdout, "brate indx 1    2    3    4    5    6    7    8    9   10   11   12   13   14\n");

	fprintf(stdout, "\n");
	exit(1);
}



/* 
  short_usage() 
  Display the short usage information
*/
void
short_usage(void)
{
	/* print a bit of info about the program */
	fprintf(stdout, "TwoLAME version %s (%s)\n", get_twolame_version(), get_twolame_url());
	fprintf(stderr, "MPEG Audio Layer II encoder\n\n");
	fprintf(stderr, "USAGE: twolame [options] <infile> [outfile]\n\n");
	fprintf(stderr, "Try \"twolame -h\" for more information.\n");
	exit(0);
}




/* 
  parse_args() 
  Parse the command line arguments
*/
void
parse_args(int argc, char **argv, twolame_options * encopts )
{

	// process args
	//static struct option longopts[] = {
	//	{ "scale",		required_argument,		NULL,			'b' },
	//	{ "scale-l",	required_argument,		NULL,			'f' },
	//	{ "scale-r",	required_argument,		NULL,		1 },
	//	{ NULL,         0,                      NULL,			0 }
	//};
	
	if (0) {
	usage();
	new_extension( NULL, NULL, NULL);
	}
}



int
main(int argc, char **argv)
{
	twolame_options	*encopts = NULL;
	FILE			*outputfile = NULL;
	short int		*pcmaudio = NULL;
	//int				num_samples = 0;
	//int				frames = 0;
	unsigned char	*mp2buffer;
	int				audioReadSize = AUDIOBUFSIZE;
	//int				mp2fill_size=0;

	if (argc == 1)
		short_usage();

	// Allocate memory for the PCM audio data
	if ((pcmaudio = (short int *) calloc(AUDIOBUFSIZE, sizeof(short int))) == NULL) {
		fprintf(stderr, "pcmaudio alloc failed\n");
		exit(99);
	}
	
	// Allocate memory for the encoded MP2 audio data
	if ((mp2buffer = (unsigned char *) calloc(MP2BUFSIZE, sizeof(unsigned char))) == NULL) {
		fprintf(stderr, "Error: mp2buffer alloc failed\n");
		exit(99);
	}
	
	// Initialise Encoder Options Structure 
	encopts = twolame_init();

	// Get options and parameters from the command line
	parse_args(argc, argv, encopts);



	// Open the input file
	
	
	// Open the output file
	if ((outputfile = fopen(outputfilename, "w")) == 0) {
		perror("failed to open output file");
		exit(2);
	}




/*	audio_info = audio_open(inputfilename, frontOptions->channels, frontOptions->samplerate);
	if (audio_info == NULL) {
		fprintf(stderr, "No input file opened.\n");
		exit(99);
	} else {
		//Use sound file to over - ride preferences for
			//mono
			/stereo and sampling - frequency
				if (audio_info->channels == 1)
				twolame_set_mode(encodeOptions, TWOLAME_MONO);

		twolame_set_num_channels(encodeOptions, audio_info->channels);
		twolame_set_in_samplerate(encodeOptions, audio_info->samplerate);
		twolame_set_out_samplerate(encodeOptions, audio_info->samplerate);
		audio_info->byteswap = frontOptions->byteswap;
	}*/
	

	/*
	 * If energy information is required, see if we're in MONO mode in
	 * which case, we only need 16 bits of ancillary data
	 */
	//if (twolame_get_energy_levels(encodeOptions))
	//	if (twolame_get_mode(encodeOptions) == TWOLAME_MONO)
	//		twolame_set_num_ancillary_bits(encodeOptions, 16);
	// only 2 bytes needed for energy level for mono channel




	// initialise twolame with this set of options
	twolame_init_params( encopts );
	
	// display file settings
	print_file_config();

	// display encoder settings
	twolame_print_config( encopts );



	if (single_frame_mode)
		audioReadSize = 1152;

	/* Now do the buffering/encoding/writing */
	//while ((num_samples = audio_info->get_samples(audio_info, pcmaudio, audioReadSize)) != 0) {
		//Read num_samples of audio data * per channel * from the input file
	//		mp2fill_size = twolame_encode_buffer_interleaved(encodeOptions, pcmaudio, num_samples, mp2buffer, MP2BUFSIZE);
	//	fwrite(mp2buffer, sizeof(unsigned char), mp2fill_size, outfile);
	//	frames += (num_samples / 1152);
	//	fprintf(stderr, "[%04i]\r", frames);
	//	fflush(stderr);
	//}

	/*
	 * flush any remaining audio. (don't send any new audio data) There
	 * should only ever be a max of 1 frame on a flush. There may be zero
	 * frames if the audio data was an exact multiple of 1152
	 */
	//mp2fill_size = twolame_encode_flush(encodeOptions, mp2buffer, MP2BUFSIZE);
	//fwrite(mp2buffer, sizeof(unsigned char), mp2fill_size, outfile);

	twolame_close(&encopts);
	

	fprintf(stderr, "\nFinished nicely\n");
	return (0);
}

