CC=g++
CFLAGS=
LDFLAGS=

all: to_raw create_mask apply_mask calc_energy


to_raw: to_raw.c
	${CC} ${CFLAGS} -lcfitsio $^ -o $@

calc_energy: calc_energy.c
	${CC} ${CFLAGS} ${LDFLAGS} $^ -o $@

apply_mask: apply_mask.c
	${CC} ${CFLAGS} ${LDFLAGS} $^ -o $@

create_mask: create_mask.c
	${CC} ${CFLAGS} ${LDFLAGS} $^ -o $@

