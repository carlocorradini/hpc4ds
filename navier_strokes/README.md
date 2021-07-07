# Navier Stokes simulations in high performance computing environment

> v.0.0.1

## Members

|     Name     |  Surname  |       Username       |    MAT     |
| :----------: | :-------: | :------------------: | :--------: |
|    Carlo     | Corradini |   `carlocorradini`   | **223811** |
| Massimiliano |  Fronza   | `massimilianofronza` | **220234** |

## Building & Compiling

### Building

> -DNO_OPEN_MP=On | Compile **without** OpenMP

```bash
$ mkdir build
$ cd build
$ cmake ..
```

### Compiling

```
$ make
```

## Run

```bash
$ mpirun -np 2 ./navierstokes --simulations=./simulations.json --results=./res --colors --loglevel=DEBUG
```

## Arguments

- --help

  Show helpful information

- --simulations=\<str>

  **REQUIRED**

  Path to JSON simulations file

- --results=\<str>

  **REQUIRED**

  Path to folder used to save JSON simulation results

- --loglevel=\<str>

  Logger level. Default to \`INFO\`

- --colors

  Enable logger output with colors

## License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details

&copy; navierstokes 2021
