#ifndef		ZWM_H_
# define	ZWM_H_

#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#define	TABLELENGTH(X) (sizeof((X)) / sizeof(*(X)))

typedef struct _Client
{
  t_elem	self;
  Window	win;
}		Client;

typedef struct	_Workspace
{
  t_list	windows;
  int		master_size;
  Bool		stacked;
  Client	*focus;
  char		*repr;
}		Workspace;

typedef struct	_ZWindow
{
  t_elem	self;
  Window	win;
}		ZWindow;

typedef struct	_Conf
{
  double	master_width;
  char		*border_focus;
  char		*border_unfocus;
}		Conf;

typedef struct	_Wm
{
  Workspace	*cur_workspace;
  Workspace	*workspaces;
  Conf		*conf;
  Display	*dpy;
  int		screen;
  Window	root;
  Bool		is_running;
  
}		Wm;

typedef union	_Arg
{
  int		val;
  const void	*args;
}		Arg;

typedef struct	_Key
{
  unsigned int	mod;
  KeySym	keysym;
  void		(*func)(const Arg *arg);
  Arg		arg;
}		Key;

/*
** Handlers
*/

void		map_request(Wm *wm, XEvent *event);

/*
** Windows
*/

void		add_window(Wm *wm, Window window);

#endif		/* !ZWM_H_ */
