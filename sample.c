#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>

int
main()
{
	static uint8_t frame[720][1280][3];
	int fds[2], x,y,t, status;
	FILE *ffmpeg;

	/* Launch ffmpeg */

	char *argv[] = {
		"ffmpeg",
		"-loglevel", "warning",
		"-stats",
		"-f", "rawvideo",
		"-pixel_format", "rgb24",
		"-video_size", "1280x720",
		"-framerate", "30",
		"-i", "-",
		"-pix_fmt", "yuv420p",
		"output.mp4",
		NULL
	};

	if (pipe(fds) == -1)
		err(1, "pipe");

	switch (fork()) {
	case -1:
		err(1, "fork");
	case 0:
		dup2(fds[0], 0);
		close(fds[1]);
		execvp("ffmpeg", argv);
		err(1, "ffmpeg");
	}

	close(fds[0]);
	ffmpeg = fdopen(fds[1], "w");

	/* Write some frames */

	for (t=0; t<300; t++) {
		memset(frame, 0, sizeof(frame));

		for (y=50; y<150; y++)
		for (x=30; x<200; x++) {
			frame[y+t][x+t][0] = 255;
			frame[y+t][x+t][1] = 255;
			frame[y+t][x+t][2] = 0;
		}

		fwrite(frame, sizeof(frame), 1, ffmpeg);
	}

	/* Close the handle and let ffmpeg finish */

	fclose(ffmpeg);

	if (wait(&status) == -1)
		err(1, "wait");
	if (status)
		errx(1, "ffmpeg exited with status %d\n", status);

	return 0;
}
