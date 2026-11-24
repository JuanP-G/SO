#ifndef CALDERO_H
#define CALDERO_H

#define M 10          /* raciones por reposicion */
#define NUMITER 3     /* veces que come cada salvaje */

#define SHM_NAME   "/caldero_shm"
#define SEM_MUTEX  "/caldero_mutex"   /* exclusion mutua sobre el caldero */
#define SEM_EMPTY  "/caldero_vacio"   /* salvaje -> cocinero: "esta vacio" */
#define SEM_FULL   "/caldero_lleno"   /* cocinero -> salvaje: "ya hay raciones" */

#endif
