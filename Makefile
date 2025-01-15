# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ltouret <ltouret@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/10/20 13:00:20 by ltouret           #+#    #+#              #
#    Updated: 2021/10/03 01:24:08 by ltouret          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ft_ping

SRCS = main.c

OBJS = ${SRCS:.c=.o}

CC		= cc
RM		= rm -f

CFLAGS = -Wall -Wextra -Werror

.c.o:
		${CC} ${CFLAGS} -c $< -o ${<:.c=.o}

$(NAME): ${OBJS}
		${CC} ${OBJS} -o ${NAME}

all:	${NAME}

clean:	${RM} ${OBJS}

fclean:	clean
		${RM} ${NAME}

re:		fclean all

test:	all
		./ft_ping 192.168.1.1

.PHONY: all clean fclean re test
