#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>

#include "lists.h"
#include "log.h"
#include "zwm.h"
#include "config.h"

/*
** Globals
*/

Wm		wm;

static void	(*handlers[LASTEvent])(Wm *wm, XEvent *event) =
{
  [ConfigureRequest] = configure_request,
  [KeyPress] = key_press,
  [MapRequest] = map_request,
  [DestroyNotify] = destroy_notify,
  [EnterNotify] = enter_notify,
  [PropertyNotify] = property_notify,
  [UnmapNotify] = unmap_notitfy,
};

/*
** Drawing
*/

void		draw(Wm *wm)
{
  t_elem	*tmp;
  Client	*client;
  Workspace	*cur = &wm->workspaces[wm->cwrksp];

  layouts[cur->layout].func(wm);
  list_foreach_as(cur->windows.head, tmp, (Client *), client)
    {
      if (client == cur->focus)
	{
	  border_width_window(wm, client, 1);
	  XSetWindowBorder(wm->dpy, client->win, wm->colors.focus);
	  XSetInputFocus(wm->dpy,client->win,RevertToParent,CurrentTime);
	  XRaiseWindow(wm->dpy,client->win);
	}
      else
	XSetWindowBorder(wm->dpy, client->win, wm->colors.unfocus);
    }
}

void		redraw(Wm *wm)
{
  undraw(wm);
  draw(wm);
}

void		undraw(Wm *wm)
{
  Workspace	*cur = &wm->workspaces[wm->cwrksp];
  t_elem	*tmp;
  Client	*client;

  list_foreach_as(cur->windows.head, tmp, (Client *), client)
    unmap_window(wm, client);
}

/*
** Event handlers
*/

void		configure_request(Wm *wm, XEvent *event)
{
  Client	*c;
  XConfigureRequestEvent *ev = &event->xconfigurerequest;
  XWindowChanges wc;

  if (!(c = get_window(wm, ev->window)))
    {
      wc.x = ev->x;
      wc.y = ev->y;
      wc.width = ev->width;
      wc.height = ev->height;
      wc.border_width = ev->border_width;
      wc.sibling = ev->above;
      wc.stack_mode = ev->detail;
      XConfigureWindow(wm->dpy, ev->window, ev->value_mask, &wc);
    }
  else
    configure(wm, c);
  XSync(wm->dpy, False);
}

void		enter_notify(Wm *wm, XEvent *event)
{
  t_elem	*tmp;
  Client	*client;
  Workspace	*cur = &wm->workspaces[wm->cwrksp];
  XCrossingEvent	enter_event;

  if (FOCUS_FOLLOWS_MOUSE == true)
    {
      enter_event = event->xcrossing;
      list_foreach_as(cur->windows.head, tmp, (Client *), client)
	{
	  if (client->win == enter_event.window)
	    {
	      cur->focus = client;
	      draw(wm);
	      break;
	    }
	}
    }
}

void		destroy_notify(Wm *wm, XEvent *event)
{
  Client	*win;

  if ((win = get_window(wm, event->xdestroywindow.window)) != NULL)
    {
      remove_window(wm, win);
      XSync(wm->dpy, False);
      draw(wm);
    }
}

void		key_press(Wm *wm, XEvent *event)
{
  unsigned	i;
  KeySym	keysym;
  XKeyEvent	keyevent;

  keyevent = event->xkey;
  keysym = XKeycodeToKeysym(wm->dpy, keyevent.keycode, 0);

  for (i = 0; i < TABLELENGTH(keys); ++i)
    if (keys[i].keysym == keysym && keys[i].mod == keyevent.state)
      keys[i].func(&keys[i].arg);
}

void		map_request(Wm *wm, XEvent *event)
{
  XMapRequestEvent *ev = &event->xmaprequest;

  if (!add_client(wm, ev->window))
    return ;
  XSelectInput(wm->dpy, event->xmaprequest.window, EnterWindowMask);
  XSync(wm->dpy, false);
  draw(wm);
}

void		property_notify(Wm *wm, XEvent *event)
{
  Client	*c;
  XPropertyEvent *ev = &event->xproperty;

  if ((c = get_window(wm, ev->window)) != NULL)
    if (ev->atom == XA_WM_HINTS)
      update_wm_hints(wm, c);
}

void		unmap_notitfy(Wm *wm, XEvent *event)
{
  Client	*client;
  XUnmapEvent	*ev = &event->xunmap;

  if ((client = get_window(wm, ev->window)) != NULL)
    remove_window(wm, client);
  draw(wm);
}

/*
** Tool functions
*/

Bool		add_client(Wm *wm, Window win)
{
  XWindowAttributes attr;
  Client	*client = NULL;
  unsigned int	save_workspace = wm->cwrksp;

  wm->cwrksp = check_rules(wm, win);
  if (!XGetWindowAttributes(wm->dpy, win, &attr))
    {
      wlog(SYS | WARN, "Can't get window's attributes");
      return (false);
    }
  if (attr.override_redirect)
    return (false);
  if (get_window(wm, win))
    return (false);
  client = add_window(wm, win);
  set_win_attributes(client, attr.x, attr.y, attr.width,
		     attr.height, attr.border_width);
  wm->cwrksp = save_workspace;
  return (true);
}

void		configure(Wm *wm, Client *c)
{
  XConfigureEvent ce;

  ce.type = ConfigureNotify;
  ce.display = wm->dpy;
  ce.event = c->win;
  ce.window = c->win;
  ce.x = c->x;
  ce.y = c->y;
  ce.width = c->width;
  ce.height = c->height;
  ce.border_width = c->border_width;
  ce.above = None;
  ce.override_redirect = False;
  XSendEvent(wm->dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

unsigned long	get_color(const char *color, Wm *wm)
{
  XColor	c;
  Colormap	map;

  map = DefaultColormap(wm->dpy, wm->screen);
  if (!XAllocNamedColor(wm->dpy, map, color, &c, &c))
    wlog(XLIB | ERR, "Bad color %s", color);
  return (c.pixel);
}

void		grab_keys(Wm *wm)
{
  size_t	i;
  KeyCode	code;

  for (i = 0; i < TABLELENGTH(keys); i++)
    {
      if ((code = XKeysymToKeycode(wm->dpy, keys[i].keysym)))
	XGrabKey(wm->dpy, code, keys[i].mod, wm->root,
		 True, GrabModeAsync, GrabModeAsync);
    }
}

void		run_wm(Wm *wm)
{
  XEvent	event;

  while (wm->is_running && !XNextEvent(wm->dpy, &event))
    {
      if (handlers[event.type])
	handlers[event.type](wm, &event);
    }
}

/* Thanks to DWM */
void		scan_windows(Wm *wm)
{
  unsigned int	i, num;
  Window	d1, d2, *wins = NULL;
  XWindowAttributes wa;

  if (XQueryTree(wm->dpy, wm->root, &d1, &d2, &wins, &num))
    {
      for (i = 0; i < num; i++)
	{
	  if(!XGetWindowAttributes(wm->dpy, wins[i], &wa)
	     || wa.override_redirect || XGetTransientForHint(wm->dpy, wins[i], &d1))
	    continue;
	  if (wa.map_state == IsViewable)
	    add_client(wm, wins[i]);
	}
      for (i = 0; i < num; i++)
	{
	  if (!XGetWindowAttributes(wm->dpy, wins[i], &wa))
	    continue;
	  if (XGetTransientForHint(wm->dpy, wins[i], &d1)
	     && (wa.map_state == IsViewable))
	    add_client(wm, wins[i]);
	}
      if (wins)
	{
	  XFree(wins);
	  draw(wm);
	}
    }
}

void		update_wm_hints(Wm *wm, Client *client)
{
  XWMHints	*wmh;

  if ((wmh = XGetWMHints(wm->dpy, client->win)))
    {
      if (wmh->flags & XUrgencyHint)
	{
	  wmh->flags &= ~XUrgencyHint;
	  XSetWMHints(wm->dpy, client->win, wmh);
	}
      XFree(wmh);
    }
}

/*
** Program life related
*/

void		free_client(void *elem)
{
  Client	*client = (Client *)elem;

  unmap_window(&wm, elem);
  XKillClient(wm.dpy, client->win);
  free(client);
}

void		finish_wm(Wm *wm)
{
  size_t	i;

  for (i = 0; i < TABLELENGTH(workspaces); i++)
    list_empty(&wm->workspaces[i].windows, free_client);
  XCloseDisplay(wm->dpy);
}

void		init_wm(Wm *wm)
{
  XSetWindowAttributes wa;

  sigchld(0);
  wm->is_running = true;
  wm->cwrksp = 0;
  wm->workspaces = workspaces;
  if ((wm->dpy = XOpenDisplay(NULL)) == NULL)
    wlog(XLIB | ERR, "Cannot open display");
  else
    wlog(XLIB | INFO, "Display Opened");
  wm->screen = DefaultScreen(wm->dpy);
  wm->root = RootWindow(wm->dpy, wm->screen);
  wm->bar_height = BAR_HEIGHT;
  wm->scr_width = DisplayWidth(wm->dpy, wm->screen);
  wm->scr_height = DisplayHeight(wm->dpy, wm->screen) - wm->bar_height;
  wm->colors.focus = get_color(FOCUS_COLOR, wm);
  wm->colors.unfocus = get_color(UNFOCUS_COLOR, wm);
  wm->layouts_number = TABLELENGTH(layouts);
  wm->layouts = layouts;
  wm->rules = rules;
  wm->rules_nb = TABLELENGTH(rules);
  grab_keys(wm);
  wa.event_mask = SubstructureNotifyMask|SubstructureRedirectMask;
  XChangeWindowAttributes(wm->dpy, wm->root, CWEventMask, &wa);
  XSelectInput(wm->dpy, wm->root, wa.event_mask);
  scan_windows(wm);
}

void		sigchld(__attribute__((unused))int unused)
{
  if(signal(SIGCHLD, sigchld) == SIG_ERR)
    wlog(SYS|ERR, "Can't install SIGCHLD handler");
  while(0 < waitpid(-1, NULL, WNOHANG));
}

int		main(void)
{
  init_wm(&wm);
  run_wm(&wm);
  finish_wm(&wm);
  return (0);
}
