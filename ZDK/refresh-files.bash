TARGETS="cab202_graphics cab202_timers cab202_sprites makefile"
CAB202=q:/cab202/2017-01
DEST=${CAB202}/ZDK 

if [ ! -d ${CAB202} ]; then mkdir ${CAB202}; fi
if [ ! -d ${DEST} ]; then mkdir ${DEST}; fi 

echo ${TARGETS}

for f in ${TARGETS}; do
	if [ -f $f ]; then cp $f $DEST; fi;
	if [ -f $f.c ]; then cp $f.c $DEST; fi;
	if [ -f $f.h ]; then cp $f.h $DEST; fi;
	if [ -f $f.html ]; then cp $f.html $DEST; fi; 	
done

if [ -d Images ]; then cp -r Images ${DEST}; fi;

cp makefile ${DEST}

echo done
