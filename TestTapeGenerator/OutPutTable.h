#ifndef OUTPUTTABLE_H
#define OUTPUTTABLE_H

const PROGMEM double Table[4][256] = {{15.98, 15.87, 15.74, 15.62, 15.49, 15.37, 15.26, 15.14, 15.02, 14.91, 14.79, 14.69, 14.58, 14.47, 14.37, 14.26, 14.16, 14.06, 13.96, 13.86, 13.75, 13.66, 13.57, 13.48, 13.39, 13.29, 13.2, 13.11, 13.03, 12.94, 12.85, 12.76, 12.68, 12.59, 12.52, 12.43, 12.35, 12.27, 12.19, 12.11, 12.03, 11.95, 11.87, 11.8, 11.72, 11.65, 11.58, 11.5, 11.43, 11.35, 11.28, 11.21, 11.14, 11.07, 11, 10.93, 10.86, 10.79, 10.73, 10.66, 10.59, 10.53, 10.46, 10.39, 10.33, 10.26, 10.2, 10.14, 10.07, 10.01, 9.94, 9.88, 9.82, 9.76, 9.7, 9.64, 9.57, 9.52, 9.45, 9.39, 9.33, 9.27, 9.22, 9.16, 9.1, 9.04, 8.98, 8.92, 8.87, 8.81, 8.75, 8.7, 8.64, 8.59, 8.53, 8.47, 8.41, 8.36, 8.3, 8.25, 8.19, 8.14, 8.08, 8.03, 7.97, 7.92, 7.86, 7.81, 7.76, 7.7, 7.65, 7.6, 7.55, 7.49, 7.44, 7.39, 7.33, 7.28, 7.23, 7.18, 7.12, 7.07, 7.02, 6.97, 6.92, 6.86, 6.81, 6.76, 6.71, 6.66, 6.61, 6.56, 6.51, 6.45, 6.4, 6.35, 6.3, 6.25, 6.2, 6.15, 6.1, 6.05, 6, 5.95, 5.9, 5.85, 5.8, 5.75, 5.7, 5.65, 5.59, 5.55, 5.5, 5.45, 5.4, 5.35, 5.3, 5.25, 5.2, 5.15, 5.09, 5.04, 4.99, 4.94, 4.89, 4.84, 4.79, 4.74, 4.69, 4.64, 4.59, 4.54, 4.49, 4.44, 4.39, 4.34, 4.29, 4.24, 4.19, 4.14, 4.08, 4.03, 3.98, 3.93, 3.89, 3.83, 3.78, 3.73, 3.68, 3.63, 3.58, 3.53, 3.47, 3.42, 3.37, 3.32, 3.27, 3.22, 3.17, 3.11, 3.06, 3, 2.96, 2.9, 2.85, 2.8, 2.75, 2.7, 2.65, 2.59, 2.54, 2.48, 2.43, 2.38, 2.32, 2.27, 2.22, 2.17, 2.11, 2.06, 2, 1.95, 1.89, 1.84, 1.78, 1.72, 1.67, 1.61, 1.56, 1.5, 1.45, 1.39, 1.33, 1.28, 1.22, 1.17, 1.1, 1.05, 0.99, 0.94, 0.88, 0.82, 0.76, 0.71, 0.65, 0.59, 0.53, 0.47, 0.41, 0.35, 0.29, 0.23, 0.17, 0.11, 0.05, 0}, {21, 20.9, 20.8, 20.6, 20.5, 20.4, 20.3, 20.2, 20, 19.9, 19.8, 19.7, 19.6, 19.5, 19.4, 19.3, 19.2, 19.1, 19, 18.9, 18.8, 18.7, 18.6, 18.5, 18.4, 18.3, 18.2, 18.1, 18, 18, 17.9, 17.8, 17.7, 17.6, 17.5, 17.4, 17.4, 17.3, 17.2, 17.1, 17, 17, 16.9, 16.8, 16.7, 16.7, 16.6, 16.5, 16.4, 16.4, 16.3, 16.2, 16.2, 16.1, 16, 15.9, 15.9, 15.8, 15.7, 15.7, 15.6, 15.5, 15.5, 15.4, 15.3, 15.3, 15.2, 15.2, 15.1, 15, 15, 14.9, 14.8, 14.8, 14.7, 14.7, 14.6, 14.5, 14.5, 14.4, 14.3, 14.3, 14.2, 14.2, 14.1, 14.1, 14, 13.9, 13.9, 13.8, 13.8, 13.7, 13.7, 13.6, 13.5, 13.5, 13.4, 13.4, 13.3, 13.3, 13.2, 13.2, 13.1, 13, 13, 12.9, 12.9, 12.8, 12.8, 12.7, 12.7, 12.6, 12.6, 12.5, 12.5, 12.4, 12.3, 12.3, 12.2, 12.2, 12.1, 12.1, 12, 12, 11.9, 11.9, 11.8, 11.8, 11.7, 11.7, 11.6, 11.6, 11.5, 11.5, 11.4, 11.4, 11.3, 11.3, 11.2, 11.2, 11.1, 11.1, 11, 11, 10.9, 10.9, 10.8, 10.8, 10.7, 10.7, 10.6, 10.6, 10.5, 10.5, 10.4, 10.4, 10.3, 10.3, 10.2, 10.2, 10.1, 10.1, 10, 10, 9.9, 9.9, 9.8, 9.8, 9.7, 9.7, 9.6, 9.6, 9.5, 9.5, 9.4, 9.4, 9.3, 9.3, 9.2, 9.2, 9.1, 9, 9, 8.9, 8.9, 8.8, 8.8, 8.7, 8.7, 8.6, 8.6, 8.5, 8.5, 8.4, 8.4, 8.3, 8.3, 8.2, 8.2, 8.1, 8.1, 8, 8, 7.9, 7.9, 7.8, 7.8, 7.7, 7.7, 7.6, 7.6, 7.5, 7.4, 7.4, 7.3, 7.3, 7.2, 7.2, 7.1, 7.1, 7, 7, 6.9, 6.9, 6.8, 6.7, 6.7, 6.6, 6.6, 6.5, 6.5, 6.4, 6.3, 6.3, 6.2, 6.2, 6.1, 6.1, 6, 6, 5.9, 5.8, 5.8, 5.7, 5.7, 5.6, 5.5, 5.5, 5.4, 5.4, 5.3, 5.2, 5.2, 5.1, 5.1, 5}, {25.9, 25.8, 25.7, 25.6, 25.4, 25.3, 25.2, 25.1, 25, 24.8, 24.7, 24.6, 24.5, 24.4, 24.3, 24.2, 24.1, 24, 23.9, 23.8, 23.7, 23.6, 23.5, 23.4, 23.3, 23.2, 23.1, 23, 23, 22.9, 22.8, 22.7, 22.6, 22.5, 22.5, 22.4, 22.3, 22.2, 22.1, 22, 22, 21.9, 21.8, 21.7, 21.7, 21.6, 21.5, 21.4, 21.4, 21.3, 21.2, 21.1, 21.1, 21, 20.9, 20.9, 20.8, 20.7, 20.7, 20.6, 20.5, 20.5, 20.4, 20.3, 20.3, 20.2, 20.1, 20.1, 20, 19.9, 19.9, 19.8, 19.8, 19.7, 19.6, 19.6, 19.5, 19.5, 19.4, 19.3, 19.3, 19.2, 19.2, 19.1, 19, 19, 18.9, 18.9, 18.8, 18.7, 18.7, 18.6, 18.6, 18.5, 18.5, 18.4, 18.3, 18.3, 18.2, 18.2, 18.1, 18.1, 18, 18, 17.9, 17.9, 17.8, 17.7, 17.7, 17.6, 17.6, 17.5, 17.5, 17.4, 17.4, 17.3, 17.3, 17.2, 17.2, 17.1, 17.1, 17, 17, 16.9, 16.9, 16.8, 16.7, 16.7, 16.6, 16.6, 16.5, 16.5, 16.4, 16.4, 16.3, 16.3, 16.2, 16.2, 16.1, 16.1, 16, 16, 15.9, 15.9, 15.8, 15.8, 15.7, 15.7, 15.6, 15.6, 15.5, 15.5, 15.4, 15.4, 15.3, 15.3, 15.2, 15.2, 15.1, 15.1, 15, 15, 14.9, 14.9, 14.8, 14.8, 14.7, 14.7, 14.6, 14.6, 14.5, 14.5, 14.4, 14.4, 14.3, 14.3, 14.2, 14.2, 14.1, 14.1, 14, 14, 13.9, 13.9, 13.8, 13.8, 13.7, 13.7, 13.6, 13.6, 13.5, 13.5, 13.4, 13.4, 13.3, 13.3, 13.2, 13.2, 13.1, 13, 13, 12.9, 12.9, 12.8, 12.8, 12.7, 12.7, 12.6, 12.6, 12.5, 12.5, 12.4, 12.4, 12.3, 12.3, 12.2, 12.2, 12.1, 12, 12, 11.9, 11.9, 11.8, 11.8, 11.7, 11.7, 11.6, 11.5, 11.5, 11.4, 11.4, 11.3, 11.3, 11.2, 11.2, 11.1, 11, 11, 10.9, 10.9, 10.8, 10.8, 10.7, 10.6, 10.6, 10.5, 10.5, 10.4, 10.3, 10.3, 10.2, 10.2, 10.1, 10, 10, 9.9}, {30.9, 30.8, 30.7, 30.6, 30.4, 30.3, 30.2, 30.1, 30, 29.9, 29.7, 29.6, 29.5, 29.4, 29.3, 29.2, 29.1, 29, 28.9, 28.8, 28.7, 28.6, 28.5, 28.4, 28.3, 28.2, 28.2, 28.1, 28, 27.9, 27.8, 27.7, 27.6, 27.5, 27.5, 27.4, 27.3, 27.2, 27.1, 27.1, 27, 26.9, 26.8, 26.8, 26.7, 26.6, 26.5, 26.5, 26.4, 26.3, 26.2, 26.2, 26.1, 26, 26, 25.9, 25.8, 25.7, 25.7, 25.6, 25.5, 25.5, 25.4, 25.3, 25.3, 25.2, 25.2, 25.1, 25, 25, 24.9, 24.8, 24.8, 24.7, 24.7, 24.6, 24.5, 24.5, 24.4, 24.3, 24.3, 24.2, 24.2, 24.1, 24.1, 24, 23.9, 23.9, 23.8, 23.8, 23.7, 23.7, 23.6, 23.5, 23.5, 23.4, 23.4, 23.3, 23.3, 23.2, 23.1, 23.1, 23, 23, 22.9, 22.9, 22.8, 22.8, 22.7, 22.7, 22.6, 22.6, 22.5, 22.4, 22.4, 22.3, 22.3, 22.2, 22.2, 22.1, 22.1, 22, 22, 21.9, 21.9, 21.8, 21.8, 21.7, 21.7, 21.6, 21.6, 21.5, 21.5, 21.4, 21.4, 21.3, 21.3, 21.2, 21.2, 21.1, 21.1, 21, 21, 20.9, 20.9, 20.8, 20.8, 20.7, 20.7, 20.6, 20.5, 20.5, 20.5, 20.4, 20.4, 20.3, 20.3, 20.2, 20.2, 20.1, 20, 20, 19.9, 19.9, 19.8, 19.8, 19.7, 19.7, 19.6, 19.6, 19.5, 19.5, 19.4, 19.4, 19.3, 19.3, 19.2, 19.2, 19.1, 19.1, 19, 19, 18.9, 18.9, 18.8, 18.8, 18.7, 18.7, 18.6, 18.6, 18.5, 18.5, 18.4, 18.4, 18.3, 18.3, 18.2, 18.2, 18.1, 18.1, 18, 18, 17.9, 17.9, 17.8, 17.8, 17.7, 17.7, 17.6, 17.5, 17.5, 17.4, 17.4, 17.3, 17.3, 17.2, 17.2, 17.1, 17.1, 17, 17, 16.9, 16.8, 16.8, 16.7, 16.7, 16.6, 16.6, 16.5, 16.5, 16.4, 16.3, 16.3, 16.2, 16.2, 16.1, 16.1, 16, 15.9, 15.9, 15.8, 15.8, 15.7, 15.7, 15.6, 15.5, 15.5, 15.4, 15.4, 15.3, 15.2, 15.2, 15.1, 15.1, 15, 15}};

#endif // OUTPUTTABLE_H
