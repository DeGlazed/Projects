#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Incorrect usage. Proper usage ./tema3 N fault\n");
        return -1;
    }

    int numtasks, rank;
    int N;
    int fault = atoi(argv[2]);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Status status;

    if (rank == 0)
    {
        N = atoi(argv[1]);
    }

    int **topology;
    topology = malloc(3 * sizeof(int *));
    int no_workers[3];

    //establish coordinators
    if (rank == 0 || rank == 1 || rank == 2)
    {
        //--------------------------------task 1
        //open files to read from
        FILE *f;
        if (rank == 0)
            f = fopen("./cluster0.txt", "r");
        else if (rank == 1)
            f = fopen("./cluster1.txt", "r");
        else
            f = fopen("./cluster2.txt", "r");

        //get number of workers form file
        fscanf(f, "%d\n", &no_workers[rank]);
        topology[rank] = malloc(no_workers[rank] * sizeof(int));
        //get the rank of each worker and store it
        for (int i = 0; i < no_workers[rank]; i++)
        {
            fscanf(f, "%d\n", &topology[rank][i]);
            //inform each worker of his coordinator
            MPI_Send(&rank, 1, MPI_INT,
                     topology[rank][i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, topology[rank][i]);
        }
        fclose(f);

        //send known topology to other two coordinators
        //number of workers and list of workers
        for (int i = 0; i < 3; i++)
        {
            if (i != rank)
            {
                MPI_Send(&no_workers[rank], 1, MPI_INT,
                         i, 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, i);
                MPI_Send(topology[rank], no_workers[rank], MPI_INT,
                         i, 1, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, i);
            }
        }
        //receive info from other two coordinators
        //number of workers and list of workers
        for (int i = 0; i < 3; i++)
        {
            if (i != rank)
            {
                MPI_Recv(&no_workers[i], 1, MPI_INT,
                         i, 0, MPI_COMM_WORLD, &status);

                topology[i] = malloc(no_workers[i] * sizeof(int));
                MPI_Recv(topology[i], no_workers[i], MPI_INT,
                         i, 1, MPI_COMM_WORLD, &status);
            }
        }

        //print topology
        printf("%d -> ", rank);
        for (int i = 0; i < 3; i++)
        {
            printf("%d:", i);
            for (int j = 0; j < no_workers[i]; j++)
            {
                if (j == no_workers[i] - 1)
                {
                    printf("%d ", topology[i][j]);
                }
                else
                {
                    printf("%d,", topology[i][j]);
                }
            }
        }
        printf("\n");

        //send topology to workers
        for (int i = 0; i < no_workers[rank]; i++) // i = worker to send the data
        {
            for (int j = 0; j < 3; j++) // j = info about which coordinator
            {
                MPI_Send(&no_workers[j], 1, MPI_INT,
                         topology[rank][i], 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, topology[rank][i]);
                MPI_Send(topology[j], no_workers[j], MPI_INT,
                         topology[rank][i], 1, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, topology[rank][i]);
            }
        }
        //--------------------------end task 1

        //------------------------task 2
        int *v;
        int data_len;
        int *cluster_data;
        int work_per_cluster[3]; //need this later for thread 0
        if (rank == 0)
        {
            /*coordinator 0 creates work tasks and sends data to other coordinators
            and his workers
            coordinator 0 receives the data and prints it*/
            //create vector to be modified
            v = malloc(N * sizeof(int));
            for (int i = 0; i < N; i++)
            {
                v[i] = i;
            }

            /*compute distributed work for clusters 
            (so that workers have comparable work to do)*/
            int elements_per_worker;
            int remaining_work;
            int total_workers = 0;
            for (int i = 0; i < 3; i++)
            {
                total_workers += no_workers[i];
            }
            elements_per_worker = N / total_workers;
            remaining_work = N % total_workers;

            //assign equal ammount of work to workers in clusters
            for (int i = 0; i < 3; i++)
            {
                work_per_cluster[i] = elements_per_worker * no_workers[i];
            }

            //distribute excess work
            int k = 0;
            while (remaining_work > 0)
            {
                //distribute excess work to each cluster
                if (remaining_work > no_workers[k])
                {
                    work_per_cluster[k] += no_workers[k];
                    remaining_work -= no_workers[k];
                }
                else
                {
                    work_per_cluster[k] += remaining_work;
                    remaining_work = 0;
                }
                //increment cluster id
                k++;
                if (k == 3)
                {
                    k = 0;
                }
            }

            //send data to cluster coordinators
            int offset = work_per_cluster[0];
            for (int i = 1; i < 3; i++)
            {
                MPI_Send(&work_per_cluster[i], 1, MPI_INT,
                         i, 0, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, i);
                MPI_Send(v + offset, work_per_cluster[i], MPI_INT,
                         i, 1, MPI_COMM_WORLD);
                printf("M(%d,%d)\n", rank, i);
                offset += work_per_cluster[i];
            }

            data_len = work_per_cluster[0];
            cluster_data = v;
        }
        else
        {
            /*coordinators 1 and 2 receive data to send to workers
            from coordinator 0*/
            MPI_Recv(&data_len, 1, MPI_INT,
                     0, 0, MPI_COMM_WORLD, &status);

            cluster_data = malloc(data_len * sizeof(int));
            MPI_Recv(cluster_data, data_len, MPI_INT,
                     0, 1, MPI_COMM_WORLD, &status);
        }

        /*compute data size for each worker
        (similar to how 0 computed data for clusters)*/
        int *data_per_worker = malloc(no_workers[rank] * sizeof(int));
        int work_per_worker = data_len / no_workers[rank];
        for (int i = 0; i < no_workers[rank]; i++)
        {
            data_per_worker[i] = work_per_worker;
        }
        int remaining_work = data_len % no_workers[rank];
        int k = 0;
        while (remaining_work > 0)
        {
            data_per_worker[k]++;
            remaining_work--;
            k++;
            if (k > no_workers[rank])
            {
                k = 0;
            }
        }

        //send data to workers
        int offset = 0;
        for (int i = 0; i < no_workers[rank]; i++)
        {
            MPI_Send(&data_per_worker[i], 1, MPI_INT,
                     topology[rank][i], 0, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, topology[rank][i]);
            MPI_Send(cluster_data + offset, data_per_worker[i], MPI_INT,
                     topology[rank][i], 1, MPI_COMM_WORLD);
            printf("M(%d,%d)\n", rank, topology[rank][i]);
            offset += data_per_worker[i];
        }

        //receive data from workers
        offset = 0;
        for (int i = 0; i < no_workers[rank]; i++)
        {
            MPI_Recv(cluster_data + offset, data_per_worker[i], MPI_INT,
                     topology[rank][i], 1, MPI_COMM_WORLD, &status);
            offset += data_per_worker[i];
        }

        if (rank == 0)
        {
            /*coordinator 0 waits for data from other 2 coordinators*/
            int offset = work_per_cluster[0];
            for (int i = 1; i < 3; i++)
            {
                MPI_Recv(v + offset, work_per_cluster[i], MPI_INT,
                         i, 1, MPI_COMM_WORLD, &status);
                offset += work_per_cluster[i];
            }

            //after receiving data, print vector content
            printf("Rezultat: ");
            for (int i = 0; i < N; i++)
            {
                printf("%d ", v[i]);
            }
            printf("\n");
        }
        else
        {
            /*coordinators 1 and 2 send data to coordinator 0*/
            MPI_Send(cluster_data, data_len, MPI_INT,
                     0, 1, MPI_COMM_WORLD);
            printf("M(%d,0)\n", rank);
        }
        //----------------------------end task 2
    }

    //workers
    else
    {
        //---------------------------------task 1
        int rank_coord;
        //receive the rank of the coordinator
        MPI_Recv(&rank_coord, 1, MPI_INT,
                 MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        //receive info about the topology
        for (int i = 0; i < 3; i++)
        {
            MPI_Recv(&no_workers[i], 1, MPI_INT,
                     rank_coord, 0, MPI_COMM_WORLD, &status);

            topology[i] = malloc(no_workers[i] * sizeof(int));
            MPI_Recv(topology[i], no_workers[i], MPI_INT,
                     rank_coord, 1, MPI_COMM_WORLD, &status);
        }

        //print topology
        printf("%d -> ", rank);
        for (int i = 0; i < 3; i++)
        {
            printf("%d:", i);
            for (int j = 0; j < no_workers[i]; j++)
            {
                if (j == no_workers[i] - 1)
                {
                    printf("%d ", topology[i][j]);
                }
                else
                {
                    printf("%d,", topology[i][j]);
                }
            }
        }
        printf("\n");
        //--------------------------------end task 1

        //--------------------------------task 2
        //receive data from coordinator
        int *recv_data;
        int data_len;
        MPI_Recv(&data_len, 1, MPI_INT,
                 rank_coord, 0, MPI_COMM_WORLD, &status);

        recv_data = malloc(data_len * sizeof(int));
        MPI_Recv(recv_data, data_len, MPI_INT,
                 rank_coord, 1, MPI_COMM_WORLD, &status);

        //modify values (in this case *2)
        for (int i = 0; i < data_len; i++)
        {
            recv_data[i] *= 2;
        }

        //send back the data
        /*no need to send back the ammount of data sent because
        coordinator knows how much he sent*/
        MPI_Send(recv_data, data_len, MPI_INT,
                 rank_coord, 1, MPI_COMM_WORLD);
        printf("M(%d,%d)\n", rank, rank_coord);
        //-------------------------------------end task 2
    }

    MPI_Finalize();
}