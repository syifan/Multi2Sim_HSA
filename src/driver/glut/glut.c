/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <assert.h>
#include <GL/glut.h>

#include <arch/x86/emu/context.h>
#include <arch/x86/emu/regs.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/string.h>
#include <mem-system/memory.h>

#include "frame-buffer.h"
#include "glut.h"


static char *glut_err_code =
	"\tAn invalid function code was generated by your application in a GLUT system\n"
	"\tcall. Probably, this means that your application is using an incompatible\n"
	"\tversion of the Multi2Sim GLUT runtime library ('libm2s-glut'). Please\n"
	"\trecompile your application and try again.\n";

static char *glut_err_one_window =
	"\tThe guest GLUT application is trying to create more than one window. Multi2Sim\n"
	"\tcurrently supports just a single GLUT window running at a time.\n";


/* Debug */
int glut_debug_category;


/*
 * GLUT calls
 */

/* List of GLUT runtime calls */
enum glut_call_t
{
	glut_call_invalid = 0,
#define X86_GLUT_DEFINE_CALL(name, code) glut_call_##name = code,
#include "glut.dat"
#undef X86_GLUT_DEFINE_CALL
	glut_call_count
};


/* List of GLUT runtime call names */
char *glut_call_name[glut_call_count + 1] =
{
	NULL,
#define X86_GLUT_DEFINE_CALL(name, code) #name,
#include "glut.dat"
#undef X86_GLUT_DEFINE_CALL
	NULL
};


/* Forward declarations of GLUT runtime functions */
#define X86_GLUT_DEFINE_CALL(name, code) \
	static int glut_abi_##name(X86Context *ctx);
#include "glut.dat"
#undef X86_GLUT_DEFINE_CALL

/* List of GLUT runtime functions */
typedef int (*glut_abi_func_t)(X86Context *ctx);
static glut_abi_func_t glut_abi_table[glut_call_count + 1] =
{
	NULL,
#define X86_GLUT_DEFINE_CALL(name, code) glut_abi_##name,
#include "glut.dat"
#undef X86_GLUT_DEFINE_CALL
	NULL
};





/*
 * GLUT global variables
 */

/* Global GLUT mutex */
pthread_mutex_t glut_mutex;

/* GLUT child thread */
pthread_t glut_thread;

/* List of events. The global GLUT mutex should be locked when accessing
 * this list. */
static struct linked_list_t *glut_event_list;

/* Flag specifying whether the host GLUT system has been initialized.
 * Lazy initialization is used to avoid error messages for X11 when
 * GLUT is not used at all. */
static int glut_initialized;




/*
 * GLUT event
 */

enum glut_event_type_t
{
	glut_event_invalid = 0,
	glut_event_display,
	glut_event_overlay_display,
	glut_event_reshape,
	glut_event_keyboard,
	glut_event_mouse,
	glut_event_motion,
	glut_event_passive_motion,
	glut_event_visibility,
	glut_event_entry,
	glut_event_special,
	glut_event_spaceball_motion,
	glut_event_spaceball_rotate,
	glut_event_spaceball_button,
	glut_event_button_box,
	glut_event_dials,
	glut_event_tablet_motion,
	glut_event_tablet_button,
	glut_event_menu_status,
	glut_event_idle,
	glut_event_timer
};

struct glut_event_t
{
	enum glut_event_type_t type;

	union
	{
		struct
		{
			int win;
		} display;

		struct
		{
			int win;
			int width;
			int height;
		} reshape;

		struct
		{
			int win;
		} overlay_display;

		struct
		{
			int win;
			unsigned char key;
			int x;
			int y;
		} keyboard;

		struct
		{
			int win;
			int button;
			int state;
			int x;
			int y;
		} mouse;

		struct
		{
			int win;
			int x;
			int y;
		} motion;

		struct
		{
			int win;
			int state;
		} visibility;

		struct
		{
			int win;
			int state;
		} entry;

		struct
		{
			int win;
			int key;
			int x;
			int y;
		} special;

		struct
		{
			int win;
			int x;
			int y;
			int z;
		} spaceball_motion;

		struct
		{
			int win;
			int x;
			int y;
			int z;
		} spaceball_rotate;

		struct
		{
			int win;
			int button;
			int state;
		} spaceball_button;

		struct
		{
			int win;
			int button;
			int state;
		} button_box;

		struct
		{
			int win;
			int dial;
			int value;
		} dials;

		struct
		{
			int win;
			int x;
			int y;
		} tablet_motion;

		struct
		{
			int win;
			int button;
			int state;
			int x;
			int y;
		} tablet_button;

		struct
		{
			int status;
			int x;
			int y;
		} menu_status;

		struct
		{
			int value;
		} timer;
	} u;
};


struct glut_event_t *glut_event_create(enum glut_event_type_t type)
{
	struct glut_event_t *event;

	/* Initialize */
	event = xcalloc(1, sizeof(struct glut_event_t));
	event->type = type;

	/* Return */
	return event;
}


void glut_event_free(struct glut_event_t *event)
{
	free(event);
}


/* Enqueue an event in the global event list. The list is accessed in a
 * thread-safe fashion by locking the global GLUT mutex. */
void glut_event_enqueue(struct glut_event_t *event)
{
	/* Lock */
	pthread_mutex_lock(&glut_mutex);

	/* Add event at tail */
	linked_list_add(glut_event_list, event);

	/* Unlock */
	pthread_mutex_unlock(&glut_mutex);
}


/* Dequeue an event from the global event list head. The list is accessed in a
 * thread-safe fashion by locking the global GLUT mutex. The function returns
 * NULL if there is no event in the queue. */
struct glut_event_t *glut_event_dequeue(void)
{
	struct glut_event_t *event;

	/* Lock */
	pthread_mutex_lock(&glut_mutex);

	/* Get event from the list of the queue */
	linked_list_head(glut_event_list);
	event = linked_list_get(glut_event_list);
	linked_list_remove(glut_event_list);

	/* Unlock */
	pthread_mutex_unlock(&glut_mutex);
	return event;
}




/*
 * GLUT host callback functions
 */

void glut_idle_func(void)
{
	struct timespec req;

	/* Flush frame buffer if needed */
	glut_frame_buffer_flush_if_requested();

	/* Suspend GLUT child thread for 100 ms */
	req.tv_sec = 0;
	req.tv_nsec = 100000000;
	nanosleep(&req, NULL);
}


void glut_display_func(void)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_display);
	event->u.keyboard.win = glutGetWindow();

	glut_event_enqueue(event);
}


void glut_overlay_display_func(void)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_overlay_display);
	event->u.keyboard.win = glutGetWindow();

	glut_event_enqueue(event);
}


void glut_reshape_func(int width, int height)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_reshape);
	event->u.reshape.win = glutGetWindow();
	event->u.reshape.width = width;
	event->u.reshape.height = height;

	glut_event_enqueue(event);
	
	/* Resize guest frame buffer */
	glut_frame_buffer_resize(width, height);

	/* Set new host OpenGL drawing values */
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width - 1, 0, height - 1);
}


void glut_keyboard_func(unsigned char key, int x, int y)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_keyboard);
	event->u.keyboard.win = glutGetWindow();
	event->u.keyboard.key = key;
	event->u.keyboard.x = x;
	event->u.keyboard.y = y;

	glut_event_enqueue(event);
}


void glut_mouse_func(int button, int state, int x, int y)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_mouse);
	event->u.mouse.win = glutGetWindow();
	event->u.mouse.button = button;
	event->u.mouse.state = state;
	event->u.mouse.x = x;
	event->u.mouse.y = y;

	glut_event_enqueue(event);
}


void glut_motion_func(int x, int y)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_motion);
	event->u.motion.win = glutGetWindow();
	event->u.motion.x = x;
	event->u.motion.y = y;

	glut_event_enqueue(event);
}


void glut_passive_motion_func(int x, int y)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_passive_motion);
	event->u.motion.win = glutGetWindow();
	event->u.motion.x = x;
	event->u.motion.y = y;

	glut_event_enqueue(event);
}


void glut_visibility_func(int state)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_visibility);
	event->u.visibility.win = glutGetWindow();
	event->u.visibility.state = state;

	glut_event_enqueue(event);
}


void glut_entry_func(int state)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_entry);
	event->u.entry.win = glutGetWindow();
	event->u.entry.state = state;

	glut_event_enqueue(event);
}


void glut_special_func(int key, int x, int y)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_special);
	event->u.special.win = glutGetWindow();
	event->u.special.key = key;
	event->u.special.x = x;
	event->u.special.y = y;

	glut_event_enqueue(event);
}


void glut_spaceball_motion_func(int x, int y, int z)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_spaceball_motion);
	event->u.spaceball_motion.win = glutGetWindow();
	event->u.spaceball_motion.x = x;
	event->u.spaceball_motion.y = y;
	event->u.spaceball_motion.z = z;

	glut_event_enqueue(event);
}


void glut_spaceball_rotate_func(int x, int y, int z)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_spaceball_rotate);
	event->u.spaceball_rotate.win = glutGetWindow();
	event->u.spaceball_rotate.x = x;
	event->u.spaceball_rotate.y = y;
	event->u.spaceball_rotate.z = z;

	glut_event_enqueue(event);
}


void glut_spaceball_button_func(int button, int state)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_spaceball_button);
	event->u.spaceball_button.win = glutGetWindow();
	event->u.spaceball_button.button = button;
	event->u.spaceball_button.state = state;

	glut_event_enqueue(event);
}


void glut_button_box_func(int button, int state)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_spaceball_button);
	event->u.button_box.win = glutGetWindow();
	event->u.button_box.button = button;
	event->u.button_box.state = state;

	glut_event_enqueue(event);
}


void glut_dials_func(int dial, int value)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_dials);
	event->u.dials.win = glutGetWindow();
	event->u.dials.dial = dial;
	event->u.dials.value = value;

	glut_event_enqueue(event);
}


void glut_tablet_motion_func(int x, int y)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_tablet_motion);
	event->u.tablet_motion.win = glutGetWindow();
	event->u.tablet_motion.x = x;
	event->u.tablet_motion.y = y;

	glut_event_enqueue(event);
}


void glut_tablet_button_func(int button, int state, int x, int y)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_tablet_button);
	event->u.tablet_button.win = glutGetWindow();
	event->u.tablet_button.button = button;
	event->u.tablet_button.state = state;
	event->u.tablet_button.x = x;
	event->u.tablet_button.y = y;

	glut_event_enqueue(event);
}


void glut_tablet_menu_status(int status, int x, int y)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_menu_status);
	event->u.menu_status.status = status;
	event->u.menu_status.x = x;
	event->u.menu_status.y = y;

	glut_event_enqueue(event);
}


void glut_timer(int value)
{
	struct glut_event_t *event;

	event = glut_event_create(glut_event_timer);
	event->u.timer.value = value;

	glut_event_enqueue(event);
}






/*
 * GLUT global functions
 */

void glut_init(void)
{
	/* Initialize GLUT global mutex */
	pthread_mutex_init(&glut_mutex, NULL);

	/* Frame buffer */
	glut_frame_buffer_init();

	/* List of events */
	glut_event_list = linked_list_create();
}

/* Forward declaration */
static struct glut_window_properties_t *glut_window_properties;
static char *glut_window_title;

void glut_done(void)
{
	struct glut_event_t *event;

	/* Kill GLUT thread */
	if (glut_thread)
		

	/* Free list of events */
	LINKED_LIST_FOR_EACH(glut_event_list)
	{
		event = linked_list_get(glut_event_list);
		glut_event_free(event);
	}
	linked_list_free(glut_event_list);

	/* Free glut window property and title */
	if (glut_window_properties)
		free(glut_window_properties);
	if (glut_window_title)
		free(glut_window_title);

	/* Free frame buffer */
	glut_frame_buffer_done();
}


int glut_abi_call(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;

	int code;
	int ret;

	/* Initialize host GLUT if it has not been done before. It is important
	 * to do a lazy initialization of the host GLUT environment, as opposed to
	 * an initialization in 'glut_init'. The reason is that a user might
	 * run 'm2s' from a remote terminal without support for X11, causing the
	 * simulator to fail even if GLUT is not used at all. */
	if (!glut_initialized)
	{
		int argc;
		char *argv[1];

		argc = 0;
		argv[0] = "m2s";
		glutInit(&argc, argv);
		glut_initialized = 1;
	}

	/* Function code */
	code = regs->ebx;
	if (code <= glut_call_invalid || code >= glut_call_count)
		fatal("%s: invalid GLUT function (code %d).\n%s",
			__FUNCTION__, code, glut_err_code);

	/* Debug */
	glut_debug("GLUT runtime call '%s' (code %d)\n",
		glut_call_name[code], code);

	/* Call GLUT function */
	assert(glut_abi_table[code]);
	ret = glut_abi_table[code](ctx);

	/* Return value */
	return ret;
}




/*
 * GLUT call #1 - init
 *
 * @param struct glut_version_t *version;
 *	Structure where the version of the GLUT runtime implementation will be
 *	dumped. To succeed, the major version should match in the runtime
 *	library (guest) and runtime implementation (host), whereas the minor
 *	version should be equal or higher in the implementation (host).
 *
 *	Features should be added to the GLUT runtime (guest and host) using the
 *	following rules:
 *	1)  If the guest library requires a new feature from the host
 *	    implementation, the feature is added to the host, and the minor
 *	    version is updated to the current Multi2Sim SVN revision both in
 *	    host and guest.
 *          All previous services provided by the host should remain available
 *          and backward-compatible. Executing a newer library on the older
 *          simulator will fail, but an older library on the newer simulator
 *          will succeed.
 *      2)  If a new feature is added that affects older services of the host
 *          implementation breaking backward compatibility, the major version is
 *          increased by 1 in the host and guest code.
 *          Executing a library with a different (lower or higher) major version
 *          than the host implementation will fail.
 *
 * @return
 *	The runtime implementation version is return in argument 'version'.
 *	The return value is always 0.
 */

#define X86_GLUT_RUNTIME_VERSION_MAJOR	1
#define X86_GLUT_RUNTIME_VERSION_MINOR	690

struct glut_version_t
{
	int major;
	int minor;
};

static int glut_abi_init(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int version_ptr;
	struct glut_version_t version;

	/* Arguments */
	version_ptr = regs->ecx;
	glut_debug("\tversion_ptr=0x%x\n", version_ptr);

	/* Return version */
	assert(sizeof(struct glut_version_t) == 8);
	version.major = X86_GLUT_RUNTIME_VERSION_MAJOR;
	version.minor = X86_GLUT_RUNTIME_VERSION_MINOR;
	mem_write(mem, version_ptr, sizeof version, &version);
	glut_debug("\tGLUT Runtime host implementation v. %d.%d\n", version.major, version.minor);

	/* Return success */
	return 0;
}




/*
 * GLUT call #2 - get_event
 *
 * The function returns the next available GLUT event.
 *
 * @param struct glut_event_t *event
 *	Pointer to a GLUT event structure where the next available event is
 *	written. If there is no new event, an event of type
 *	'glut_event_idle' is returned.
 *
 * @return
 *	The return value is always 0.
 */

static int glut_abi_get_event(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	struct glut_event_t *event;
	struct glut_event_t idle_event;

	unsigned int event_ptr;

	/* Read arguments */
	event_ptr = regs->ecx;
	glut_debug("\tevent_ptr=0x%x\n", event_ptr);

	/* Get event at the head of the queue. If there is no event, create an
	 * event of type 'idle'. */
	event = glut_event_dequeue();
	if (event)
	{
		mem_write(mem, event_ptr, sizeof(struct glut_event_t), event);
		glut_event_free(event);
	}
	else
	{
		memset(&idle_event, 0, sizeof(struct glut_event_t));
		idle_event.type = glut_event_idle;
		mem_write(mem, event_ptr, sizeof(struct glut_event_t), &idle_event);
	}

	/* Return event */
	return 0;
}



/*
 * GLUT call #3 - new_window
 *
 * @param char *title
 *	Title of the window.
 * @param struct glut_window_properties_t *properties
 *	Pointer to a structure describing the window to be created.
 *
 * @return
 *	The host ID of the window, as returned by the GLUT host call.
 */

struct glut_window_properties_t
{
	int x;
	int y;
	int width;
	int height;
};

static struct glut_window_properties_t *glut_window_properties;
static char *glut_window_title;


/* GLUT child thread function.
 * The argument of the function is ignored.
 * The function takes the allocated values for 'glut_window_properties' and
 * 'glut_window_title' from the thread creator, uses them, and then frees them. */
static void *glut_thread_func(void *arg)
{
	struct glut_window_properties_t *properties = glut_window_properties;
	char *title = glut_window_title;

	/* Detach thread. Parent does not need to 'pthread_join' the child to
	 * release its resources. */
	pthread_detach(pthread_self());

	/* Configure host window properties */
	glutInitWindowPosition(properties->x, properties->y);
	glutInitWindowSize(properties->width, properties->height);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

	/* Create window */
	glutCreateWindow(title);

	/* Host window callbacks */
	glutIdleFunc(glut_idle_func);
	glutDisplayFunc(glut_display_func);
	glutOverlayDisplayFunc(glut_overlay_display_func);
	glutReshapeFunc(glut_reshape_func);
	glutKeyboardFunc(glut_keyboard_func);
	glutMouseFunc(glut_mouse_func);
	glutMotionFunc(glut_motion_func);
	glutPassiveMotionFunc(glut_passive_motion_func);
	glutVisibilityFunc(glut_visibility_func);
	glutEntryFunc(glut_entry_func);
	glutSpecialFunc(glut_special_func);
	glutSpaceballRotateFunc(glut_spaceball_rotate_func);
	glutSpaceballButtonFunc(glut_spaceball_button_func);
	glutButtonBoxFunc(glut_button_box_func);
	glutDialsFunc(glut_dials_func);
	glutTabletMotionFunc(glut_tablet_motion_func);
	glutTabletButtonFunc(glut_tablet_button_func);

	/* Resize guest frame buffer */
	glut_frame_buffer_resize(properties->width, properties->height);

	/* Free input arguments */
	free(properties);
	free(title);
	glut_window_properties = NULL;
	glut_window_title = NULL;

	/* Host GLUT main loop */
	glutMainLoop();

	/* Function never returns */
	return NULL;
}

static int glut_abi_new_window(X86Context *ctx)
{
	struct x86_regs_t *regs = ctx->regs;
	struct mem_t *mem = ctx->mem;

	unsigned int title_ptr;
	unsigned int properties_ptr;

	char title[MAX_STRING_SIZE];

	struct glut_window_properties_t *properties;

	/* Read arguments */
	title_ptr = regs->ecx;
	properties_ptr = regs->edx;
	mem_read_string(mem, title_ptr, sizeof title, title);

	/* Create window properties and read */
	properties = xcalloc(1, sizeof(struct glut_window_properties_t));
	mem_read(mem, properties_ptr, sizeof(struct glut_window_properties_t),
		properties);

	/* Debug */
	glut_debug("\ttitle_ptr=0x%x, properties_ptr=0x%x\n",
		title_ptr, properties_ptr);
	glut_debug("\ttitle='%s'\n", title);
	glut_debug("\tx=%d, y=%d, width=%d, height=%d\n", properties->x,
		properties->y, properties->width, properties->height);
	
	/* Check that only one GLUT window is created */
	if (glut_thread)
		fatal("%s: only one GLUT window allowed.\n%s",
			__FUNCTION__, glut_err_one_window);

	/* Store values for 'glut_window_properties' and 'glut_window_title',
	 * consumed by the secondary thread that creates the window. */
	glut_window_properties = properties;
	glut_window_title = xstrdup(title);

	/* Launch secondary thread for host GLUT calls. A secondary thread is needed
	 * to cover the limitations of glutMainLoop, a function that does not return. */
	if (pthread_create(&glut_thread, NULL, glut_thread_func, NULL))
		fatal("%s: could not create child thread", __FUNCTION__);

	/* Return host GLUT window ID. */
	return 1;
}




/*
 * GLUT call #4 - test_draw
 *
 * Draw a test black-to-white faded pattern horizontally in the frame buffer,
 * and flush it to the currently active host GLUT window.
 *
 * @return
 *	The function always returns 0
 */

static int glut_abi_test_draw(X86Context *ctx)
{
	int width;
	int height;
	int color;
	int gray;

	int i;
	int j;

	/* Clear frame buffer */
	glut_frame_buffer_clear();

	/* Draw pattern */
	glut_frame_buffer_get_size(&width, &height);
	for (i = 0; i < width; i++)
	{
		gray = (double) i / width * 0xff;
		color = (gray << 16) | (gray << 8) | gray;
		for (j = 0; j < height; j++)
			glut_frame_buffer_pixel(i, j, color);
	}

	/* Refresh host GLUT window */
	glut_frame_buffer_flush_request();

	/* Return */
	return 0;
}

