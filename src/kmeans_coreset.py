import zmq
from nilearn.input_data import NiftiMasker
import pandas as pd
import numpy as np
import glob
import os
from sklearn.cluster import kmeans
import nibabel as nib

def compute_outliers_FD(target_file, source, threshold):
    mvt_file = pd.read_csv(target_file, header=None, sep='  ') 
    if source=='FSL':
        mvt_file = mvt_file.rename(columns={0: 'Rx', 1: 'Ry', 2: 'Rz', 3: 'Tx', 4: 'Ty', 5: 'Tz'})
    else:
        mvt_file = mvt_file.rename(columns={3: 'Rx', 4: 'Ry', 5: 'Rz', 0: 'Tx', 1: 'Ty', 2: 'Tz'})
    mvt_file[['Rx', 'Ry', 'Rz']] = mvt_file[['Rx', 'Ry', 'Rz']]*180/np.pi
    diff_mvt = mvt_file.diff()
    diff_mvt.iloc[0]=[0,0,0,0,0,0]
    diff_sphere = diff_mvt
    diff_sphere[['Rx', 'Ry', 'Rz']] = diff_sphere[['Rx', 'Ry', 'Rz']]*50*np.pi/180
    return diff_sphere.abs().sum(axis=1) > threshold

def reset_socket(socket):
    socket.close()            
    socket=context.socket(zmq.REQ)
    socket.connect("tcp://localhost:5555")

import struct
def send_array(socket, array):
    socket.send(array.tobytes())
    message=socket.recv()
    val=struct.unpack('b', message)[0]
    if val != 5:
        raise NameError('Unexpected response from server')

def reduce_array(socket, array):
    for f in array:
        send_array(socket, np.insert(f,0,0))

            
def get_representatives(socket):
    reps = []
    socket.send(np.asarray([11.0]))
    message = socket.recv()
    payload = np.frombuffer(message, dtype=np.double())
    if payload[0] == 6.0:
        socket.send(np.asarray([5.0]))
        for i in range(0, int(payload[1])):
            message=socket.recv()
            payload = np.frombuffer(message, dtype = np.double())
            reps.append(payload)
            socket.send(np.asarray([5.0]))
        message = socket.recv()
    return np.vstack(reps)


def get_array_representatives(array, socket):
    reduce_array(socket, array)
    reps = get_representatives(socket)
    
def prepare_confounds(mvt_path, csf_path, wm_path, gm_path, compute_mvt_derivatives=True, compute_mvt_squares=True):
    df_orig = pd.read_csv(mvt_path, header=None, sep='  ')
    df_data = df_orig.to_numpy()
    total_data = df_data.copy()
    if csf_path is not None:
        csf_signal = pd.read_csv(csf_path, header=None)
        total_data = np.hstack((csf_signal.to_numpy(), total_data))
    if wm_path is not None:
        wm_signal = pd.read_csv(wm_path, header=None)
        total_data = np.hstack((wm_signal.to_numpy(), total_data))
    if gm_path is not None:
        gm_signal = pd.read_csv(gm_path, header=None)
        total_data = np.hstack((gm_signal.to_numpy(), total_data))
    if compute_mvt_derivatives:
        derivatives = np.gradient(df_data)[0]
        total_data = np.hstack((total_data, derivatives))
    if compute_mvt_squares:
        total_data = np.hstack((total_data, df_data**2))
        if compute_mvt_derivatives:
            total_data = np.hstack((total_data, derivatives**2))
    all_confounds = pd.DataFrame(total_data)
    return all_confounds
    
# We would like for each assignement to obtain in what 'state' we were.
# To gather statistics, basically.
import seaborn as sns

def gather_stats_and_plot(hrf_delay, events, k, assignements, x):
    """
    Given an input array of events, along with an input array of CAPs assignement and their associated time array x, the number of CAPs and an HRF delay:
    - Assign to each event the assignement that is closest in time (ie: assign the time t that is closest to event time + hrf_delay)
    - Aggregate, per CAP, the different events from perspective of behavioural response (error type basically)
    - Lastly, plot the results, displaying frequency of each error type per CAP.
    :param hrf_delay: 
    :param events: 
    :param k: 
    :param assignements: 
    :param x: 
    :return: 
    """
    gathered_stats = np.zeros((4, k))
    get_array_entry = lambda i : (int(events.iloc[i]['trial_type']=='go')<<1) + int(events.iloc[i]['resp_correct'])
    for i in range(0, events.shape[0]-1):
        time = int(round(events.iloc[i]['onset'] + hrf_delay))
        corresp = np.where(x == time)
        if corresp[0].size == 0:
            print('Time {} not found'.format(time))
        centroid = assignements[np.where(x == time)]
        gathered_stats[get_array_entry(i), centroid] += 1
    cap_arr = []
    gathered_stats /= gathered_stats.sum(axis=0)
    for i in range(0,k):
        cap_arr.extend(['CAP_' + str(i)]*4)
    df=pd.DataFrame({'Frequency': gathered_stats.flatten(order='F'), 'Error Type': ['Commission Error', 'Correct Omission', 'Omission Error', 'Correct Commission']*k, 'CAP': cap_arr})
    sns.catplot(x='Error Type', hue='CAP', y='Frequency', data=df, kind='bar')
    plt.show()



def make_path(subject_name, session_name):
    return os.path.join('/home/guibertf/Documents/Freya/2021-Esterman_InOutZone/niftifiles_GVA_2020/derivatives/', 'analysis', subject_name, session_name, 'func', subject_name + '_' + session_name + '_task-gradCPT_bold_masked-fullbrain_zscored_smoothed-5mm_detrended_csf-removed_wm-removed_8volumes-scrubbed.npy')



def run_k_means_and_get_assignements(k, reps, sessions):
    # Get clusters for a specific k
    # Then we can run kmeans on this reduced version
    kmeans = KMeans(n_clusters=k) # some k. A relevant question (but to be answered later) is whether consensus clustering might be relevant here or not
    kmeans.fit(reps) # compute centroids on reduced data version
    # Important: save the CAPs that were found, so they can be inspected.
    masker = NiftiMasker(mask_img='/usr/local/fsl/data/standard/MNI152_T1_2mm_brain_mask_dil1.nii.gz')
    m = nib.load(masker.mask_img)
    for i in range(0,k):
        data = m.get_fdata()
        data[m.get_fdata().astype(bool)] = reps[i]
        nib.save(nib.Nifti1Image(data, m.affine, m.header), '/home/guibertf/Documents/Freya/2021-Esterman_InOutZone/niftifiles_GVA_2020/derivatives/analysis/sub-07/CAPS_' + str(i) + '.nii.gz')
    assignements = []
    # Now we want to get the assignments for all sessions, using these centroids. To do so, we must reload the data (a tad painful)
    for sess in sessions:
        session_name = sess.split('/')[-2]
        save_path = make_path(subject_name, session_name)
        # To get the session, load it from disk
        session_data = np.load(save_path)
        # Get back motion outliers
        mvt_file = glob.glob(os.path.join(sess, 'func','sub*gradCPT*_mcf.txt'))
        mvt_file = mvt_file[0]
        outliers = compute_outliers_FD(mvt_file, 'FSL', 0.5)[8:]
        # Predict only on volumes with decent motion
        assignements.append(kmeans.predict(session_data[:,:][np.logical_not(outliers),:].astype(np.double())))
        del session_data # We still care about the RAM !
    return assignements, kmeans

# Get statistics for that k and plot it

def get_event_type_id(event):
    return 2*int(event['trial_type']=='go') + int(event['resp_correct'])
    
    


def gather_stats_for_session(sess_assignements, k, sess_events, hrf_delay=5.0):
    times = np.asarray(range(8, 788))
    transition_stats = np.zeros((4, k, k))
    for e in range(0, sess_events.shape[0]):
        t = sess_events.iloc[e]['onset'] + hrf_delay
        t_low = int(t)
        t_up = t_low + 1
        id_up = np.where(times==t_up)[0]
        id_low = np.where(times==t_low)[0]
        if id_up.size == 0 or id_low.size == 0:
            print('No corresp for time interval {}-{}, one of the bounds is missing in EPI time'.format(t_low, t_up))
        else:
            # Get event type and increment count of transition
            transition_stats[get_event_type_id(sess_events.iloc[e]),sess_assignements[id_low], sess_assignements[id_up]] += 1
    return transition_stats


# Open the socket to communicate
context = zmq.Context()
socket=context.socket(zmq.REQ)
socket.connect("tcp://localhost:5555")

# We will worry about subject 07 for now, because it has no motion outliers.
subject_path = '/home/guibertf/Documents/Freya/2021-Esterman_InOutZone/niftifiles_GVA_2020/derivatives/preprocessed/sub-07/'
subject_name = subject_path.split('/')[-2]
sessions = sorted(glob.glob(os.path.join(subject_path, 'ses*/')), key=lambda x: int(x.split('-')[3][:-1]))


# Now that various sessions have been 'reduced' we lack the centroids. We will compute them in two steps:
# Get the various sessions
for sess in sessions:
    session_name = sess.split('/')[-2]
    # Load the data, preprocess it, so on and so forth
    mvt_file = glob.glob(os.path.join(sess, 'func','sub*gradCPT*_mcf.txt'))
    if len(mvt_file) > 0:
        mvt_file = mvt_file[0]
        outliers = compute_outliers_FD(mvt_file, 'FSL', 0.5)[8:]
        # Prepare the confounds to be regressed out: movement parameters, csf signal, white matter signal
        confounds = prepare_confounds(mvt_path=mvt_file, csf_path= glob.glob(os.path.join(sess, 'func','sub*gradCPT*_csf.txt'))[0], wm_path= glob.glob(os.path.join(sess, 'func','sub*gradCPT*_wm.txt'))[0], gm_path=None)
        # Session data will be zscored, smoothed (5 fwhm mm), linearly detrended. 
        # To further spare RAM, the input data is restricted to a brain mask, which encompasses entire brain. To minimize odds of throwing away relevant information, we use a dilated version
        masker = NiftiMasker(mask_img='/usr/local/fsl/data/standard/MNI152_T1_2mm_brain_mask_dil1.nii.gz', standardize='zscore', smoothing_fwhm=5, t_r=1, detrend=True)
        # Apply the processing pipeline to our input data, at last c:
        session_data = masker.fit_transform(glob.glob(os.path.join(sess, 'func','sub*gradCPT_bold_mni.nii.gz'))[0],confounds=confounds)[8:,:].astype(np.double())
        # We restrict the reduciton only to positions that are NOT motion outliers
        reduce_array(socket, session_data[np.logical_not(outliers),:]) # Careful here to use indeed float64, because C++ expects under the hood to have arrays of doubles!
        # We might want to save these results to disk, instead of recomputing them later down the line.
        save_path = make_path(subject_name, session_name)
        np.save(save_path, session_data, allow_pickle=False)
        del session_data # deallocate it to save our oh so dear RAM !
        del confounds
        del masker
    else:
        print('No file :(')


# Now we get the representatives
reps = get_representatives(socket)

# Now get the centroids
assignements, km =  run_k_means_and_get_assignements(2, reps, sessions)

# Now we can start to dig a bit into the assignements compared to behavioural data

behavioural_path = os.path.join('/'.join(subject_path.split('/')[:-4]), subject_name)
behav_sessions = sorted(glob.glob(os.path.join(behavioural_path, 'ses*/')), key=lambda x: int(x.split('-')[3][:-1]))

import matplotlib.pyplot as plt

for i in range(0, len(behav_sessions)):
    transition_stats = gather_stats_for_session(assignements[i], 2, pd.read_csv(glob.glob(os.path.join(behav_sessions[i], 'func', '*gradCPT_events.tsv'))[0], delimiter='\t'))
    norm_freqs = transition_stats / transition_stats.sum(axis=0)
    df=pd.DataFrame({'Frequency': norm_freqs.flatten(order='C'), 'Type': ['No go incorrect']*k*k + ['No go correct']*k*k + ['Go incorrect']*k*k + ['Go correct']*k*k, 'Transition': ['CAP ' + str(j) + '->CAP ' + str(i) for i in range(0, 2) for j in range(0, 2)]*4})
    sns.catplot(y='Frequency', x='Transition', hue='Type', kind='bar', data=df)
    plt.show()



for sess in behav_sessions:
    events=pd.read_csv(glob.glob(os.path.join(sess, 'func', '*gradCPT_events.tsv'))[0], delimiter='\t')
    go_trials=events[events['trial_type']=='go']
    no_go_trials=events[events['trial_type']=='no-go']
    x = np.asarray(range(8, 788))
    hrf_delay=5.0
    fig, axs = plt.subplots(2)
    fig.suptitle('Brain pattern (top) and behavioural patterns (bottom)')
    for i in range(0, k):
        axs[0].plot(x[assignements[0]==i]-hrf_delay, assignements[0][assignements[0]==i], 'o', label='CAPs ' + str(i))
    axs[0].legend()
    axs[0].set(xlabel='Time (s)')
    axs[1].plot(go_trials[go_trials['resp_correct'] == 1]['onset'], [2]*go_trials[go_trials['resp_correct'] == 1].shape[0], 'o', label='commission correct')
    axs[1].plot(go_trials[go_trials['resp_correct'] == 0]['onset'], [3]*go_trials[go_trials['resp_correct'] == 0].shape[0], 'o', label='omission incorrect')
    axs[1].plot(no_go_trials[no_go_trials['resp_correct']==1]['onset'], [0]*no_go_trials[no_go_trials['resp_correct']==1].shape[0], 'o', label='omission correct')
    axs[1].plot(no_go_trials[no_go_trials['resp_correct']==0]['onset'], [1]*no_go_trials[no_go_trials['resp_correct']==0].shape[0], 'o', label='commission incorrect')
    axs[1].legend()
    axs[1].set(xlabel='Time (s)')
    plt.show()



