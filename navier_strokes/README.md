# Navier Stokes simulations in high performance computing environment

> v.0.0.1

## Members

|     Name     |  Surname  |       Username       |    MAT     |
| :----------: | :-------: | :------------------: | :--------: |
|    Carlo     | Corradini |   `carlocorradini`   | **223811** |
| Massimiliano |  Fronza   | `massimilianofronza` | **220234** |

## Building & Compiling

### Building

> -DNO_OPEN_MP=On to compile **without** OpenMP

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
$ mpirun -n <#> ./navierstokes --simulations=./simulations.json --colors --loglevel=DEBUG
```

## Arguments

- --help

  Show helpful information

- --simulations=<str>

  Path to JSON simulations file

- --loglevel=<str>

  Logger level. Default to \`INFO\`

- --colors

  Enable logger output with colors

## License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details

&copy; navierstokes 2021
