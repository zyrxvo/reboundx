import rebound
import reboundx
import numpy as np
from sys import argv

twopi = 2.*np.pi
tmax = twopi * 2e9  # Maximum integration time.
n = int(argv[1]) # Simulation number.

taus = np.insert(np.logspace(7, np.log10(2e9), 24), 0, 0) # Assuming 24 simulations, with taus that span from 1e7 yrs to 2e9 yrs.
tau = twopi * np.sign(n) * taus[np.abs(n)] # Rate of change in c.

folder = 'archives/'
if n == 0:
    filename = folder + 'gr_0.bin'
else:
    filename = folder + 'gr_' + ('m' if n<0 else 'p') + str(abs(n)) + '.bin'

try:
    sim = rebound.Simulation(filename)
    sim.automateSimulationArchive(filename, step=int(2e5*twopi/sim.dt), deletefile=False) # save snapshots every 200,000 years
    print("Continuing from {0:8.3f} Myrs".format(sim.t/twopi/1e6))
except:
    try:
        sim = rebound.Simulation('ss.bin')
    except:
        sim = rebound.Simulation()
        sim.add(['sun', 'mercury', 'venus', 'earth', 'mars', 'jupiter', 'saturn', 'uranus', 'neptune'], date='2000-01-01 12:00')
        sim.save('ss.bin')

    sim.move_to_com()
    sim.integrator = 'whckl'
    sim.dt = np.sqrt(37)*twopi/365.25 # Timestep of about 6 days.
    sim.ri_whfast.safe_mode = 0
    sim.ri_whfast.keep_unsynchronized = True
    sim.exit_min_distance = 3e-3 # Distance between the Earth and Moon.
    sim.exit_max_distance = 1000. # Unlikely that the planet is bound.
    sim.automateSimulationArchive(filename, step=int(2e5*twopi/sim.dt), deletefile=True) # save snapshots every 200,000 years

rebx = reboundx.Extras(sim)
gr = rebx.load_force('gr_potential')
rebx.add_force(gr)
gr.params['c'] = tau # Set the rate of change in c.

print('Start integrating.')
try:
    sim.integrate(tmax, exact_finish_time=False)
except rebound.Escape as esc:
    print(esc)
except rebound.Encounter as enc:
    print(enc)
print('Finished integrating.')
