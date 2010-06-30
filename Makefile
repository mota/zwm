TYPE		=	$(shell uname -s)
TYPE		!=	uname -s

NAME		=	zwm

CC		=	gcc
CD		=	cd
MAKE		=	make
CP		=	cp -r
RM		=	rm -rf
FIND		=	find

CFLAGS		+=	-O2 -Werror -DNDEBUG
#CFLAGS		=	-g3
CFLAGS		+=	-W -Wall -ansi -pedantic
CFLAGS		+=	-D${TYPE} -D_REENTRANT -D__EXTENSIONS__
CFLAGS		+=	-D_POSIX_PTHREAD_SEMANTICS -D_BSD_SOURCE -D_GNU_SOURCE

CFLAGS		+=	-I$(HDRDIR) -I/usr/X11R6/include

LDFLAGS		=	-L/usr/X11R6/lib -lX11

SRCDIR		=	src
HDRDIR		=	include


SRCS		=

OBJS		=	$(SRCS:.c=.o)

HDRS		=

$(NAME):		$(SRCS) $(OBJS) $(HDRS)
			$(CC) $(OBJS) -o $(NAME) $(LDFLAGS)

all:			$(NAME)

clean:
			$(RM) $(OBJS)
			$(RM) $(LIBDIR)/*.a
			$(FIND) . -name "*~" -print -exec rm -f {} \;
			$(FIND) . -name "*\#*" -print -exec rm -f {} \;
			$(RM) $(NAME).core
			$(RM) vgcore*

fclean:			clean
			$(RM) $(NAME)
			$(RM) TAGS

mrproper:		$(NAME) clean

re:			fclean $(NAME)

.c.o:
			$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

.PHONY:			clean all fclean mrproper re
