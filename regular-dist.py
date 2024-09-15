import numpy

TOTAL_NUM_PARTICLES=30000
FRACTION_SPECIES=0.1
CUBE_DIM=TOTAL_NUM_PARTICLES**(1.0/3.0)
FRAC_STD_DEV = 1.0
FRACTIONAL_EXTENT_OF_DEVIATION = 1.0
PARTICLE_DIAMETER = 1.0
BOND_LENGTH = 0.9
numMolecules = TOTAL_NUM_PARTICLES * FRACTION_SPECIES
SEED= 12357

rng1 = numpy.random.default_rng(SEED)

def calcSpacing():
    total_volume = CUBE_DIM * CUBE_DIM * CUBE_DIM
    molecule_volume_increment = total_volume/numMolecules
    mean_spacing = molecule_volume_increment**(1.0/3.0)
    return mean_spacing


def truncatedNormal(mean,std_dev):

    val = 0.0
    while (val - mean < BOND_LENGTH):
        val = numpy.random.normal(mean, std_dev)

    return val


def randomDrift(mean,upper_limit):

    offset =  rng1.uniform(-1.0,1.0) *upper_limit

    return mean+offset



def calcCoordinates():
    spacing = calcSpacing()
    num_steps = int(round(CUBE_DIM/spacing,0))
    std_dev = spacing*FRAC_STD_DEV
    upper_limit = (spacing -PARTICLE_DIAMETER)/2.0 * FRACTIONAL_EXTENT_OF_DEVIATION
    percent_deviation = 100.0 * upper_limit/spacing 

   # outfile = open("M30000-exposed--regular_dist-frac_ext_dev-1_0.csv",'w')
    outfile = open("M30000-PAG-regular_dist-frac_ext_dev-1_0.csv",'w')

    outfile.write("# spacing={0}, upper_limit={1}, FRACTIONAL_EXTENT_OF_DEVIATION={2}, percent_deviation={3}\n".format(spacing,upper_limit,FRACTIONAL_EXTENT_OF_DEVIATION,percent_deviation))
    outfile.write("x,y,z,monomer_name,monomer_id,chain_index\n")

    pag_index = 1

    nominal_x_coord = spacing/2

    while nominal_x_coord < CUBE_DIM:
        nominal_y_coord = spacing/2
        while nominal_y_coord < CUBE_DIM:
            nominal_z_coord = spacing/2
            while nominal_z_coord < CUBE_DIM:
                # x = truncatedNormal(nominal_x_coord, std_dev)
                # y = truncatedNormal(nominal_y_coord, std_dev)
                # z = truncatedNormal(nominal_z_coord, std_dev)
                x = randomDrift(nominal_x_coord, upper_limit)
                y = randomDrift(nominal_y_coord, upper_limit)
                z = randomDrift(nominal_z_coord, upper_limit)             
 #               outfile.write("{0},{1},{2},TBMA,3,1\n".format(x,y,z))
#                outfile.write("{0},{1},{2},MAA,7,1\n".format(x,y,z))
                outfile.write("{0},{1},{2},PAG,5,1\n".format(x,y,z))
                pag_index += 1
                nominal_z_coord += spacing
            nominal_y_coord += spacing
        nominal_x_coord += spacing

    outfile.close()
    print("processed molecules: " + str(pag_index))



calcCoordinates()
