% For this demo to work, you should be running the coreset server. To do
% so, go to the treeCoreset project. Open a terminal there, and type
% ./build
% cd build
% ./treeCoreset
%
% For more informations on the available options, run
% ./treeCoreset --help
%
% You can interrupt the server at any time by closing the terminal.
% Note that the server MUST be running for point transmission (see below)
% to work.
%

addpath(genpath('network/'))

%% If the mex files are not compiled, compile them using mex
if(isempty(dir('network/sendPoint.mex*')))
    mex -O network/sendPoint.cpp network/ClientMessaging.cpp network/MessagingUtils.cpp -lzmq -outdir network
end

if(isempty(dir('network/stopServer.mex*')))
    mex -O network/stopServer.cpp network/ClientMessaging.cpp network/MessagingUtils.cpp -lzmq -outdir network
end

if(isempty(dir('network/getKclusters.mex*')))
    mex -O network/getKclusters.cpp network/ClientMessaging.cpp network/MessagingUtils.cpp -lzmq -outdir network
end

if(isempty(dir('network/getRepresentatives.mex*')))
    mex -O network/getRepresentatives.cpp network/ClientMessaging.cpp network/MessagingUtils.cpp -lzmq -outdir network
end
%% Read in some brain data

patient_names = {'13', '14', '15', '16', '17', '18', '19', '20'};
mask_path = ['/home/guibertf/Documents/Freya/2021-Esterman_InOutZone/niftifiles_GVA_2020/derivatives/preprocessed/sub-13/ses-01/func/MNI152_T1_2mm_brain_mask.nii'];
brain = logical(spm_read_vols(spm_vol(mask_path)));

% The first step is to read once every volume to get the NaNs.
% The NaN mask is the intersection of all NaN masks.
n_voxels = sum(sum(sum(brain)));
non_nan_mask = logical(ones(n_voxels,1));

for i=1:size(patient_names,2)
    p_num = patient_names{i};
    file_path = ['/home/guibertf/Documents/Freya/2021-Esterman_InOutZone/niftifiles_GVA_2020/derivatives/preprocessed/sub-' p_num '/ses-01/func/sub-' p_num '_ses-01_task-gradCPT_bold_mni.nii'];
    if(isempty(dir(file_path)) && ~isempty(dir([file_path '.gz'])))
       % Extract the file with gunzip
       gunzip([file_path '.gz']);
    end
    if(~isempty(dir(file_path)) && ~isempty(dir(mask_path)))
        disp(['Reading volume: ' file_path]);
        disp(['Using mask: ' mask_path]);
        V=niftiread(file_path);
        n_points=size(V,4);

        V_vec = zeros(n_voxels,n_points, 'double');

        for i=1:n_points
            vol=V(:,:,:,i);
            V_vec(:,i)=vol(brain(:));
        end
        % Standardize the session and select only non NaN positions
        mu = mean(V_vec, 2);
        sig = std(V_vec, 0, 2);
        V_vec = V_vec - mu;
        V_vec = V_vec*1./sig;
        non_nan_mask = non_nan_mask & ~isnan(V_vec(:,1));
        clear V V_vec vol mu sig;
    else
       disp(['No file found for ' p_num]);
    end
end

%%
disp('Done');
%%
% Start over
for i=1:size(patient_names,2)
    p_num = patient_names{i};
    file_path = ['/home/guibertf/Documents/Freya/2021-Esterman_InOutZone/niftifiles_GVA_2020/derivatives/preprocessed/sub-' p_num '/ses-01/func/sub-' p_num '_ses-01_task-gradCPT_bold_mni.nii'];
    if(~isempty(dir(file_path)) && ~isempty(dir(mask_path)))
        disp(['Reading volume: ' file_path]);
        disp(['Using mask: ' mask_path]);
        V=niftiread(file_path);
        n_points=size(V,4);

        V_vec = zeros(n_voxels,n_points, 'double');

        for i=1:n_points
            vol=V(:,:,:,i);
            V_vec(:,i)=vol(brain(:));
        end
        clear V vol;
        % Standardize the session and select only non NaN positions
        mu = mean(V_vec, 2);
        sig = std(V_vec, 0, 2);
        V_vec = V_vec - mu;
        V_vec = V_vec*1./sig;
        
        % Now send all of these to the representation
        disp('Starting to send points for this session');

        for i=1:n_points
            sendPoint(V_vec(non_nan_mask, i));
        end
        
        clear V_vec mu sig;
        disp('Sent all points');
    else
       disp(['No file found for ' p_num]);
    end
end


%% Now we can get the representative points from the cluster very easily!
mycellreps = cell(1,500);
[mycellreps{:}] = getRepresentatives();

%% We can also ask the server to perform directly kmeans on server-side.
% In this case kmeans is repeated several times to get more stable
% solution.
mycell = cell(1,2);
[mycell{:}]=getKclusters(2); % This showcases how to get here two clusters

%% In any case, once we are done with the coreset, stop it, so that we can reclaim all memory cleanly.
stopServer()

%% Now format the representatives and perform, e.g, consensus clustering on these representatives instead of the full data
% This leads to a decrease in computational time, since we're dealing with
% less data.

% Put representatives as a single matrix
representatives = cell2mat(cellfun(@(x) x', mycellreps, 'UniformOutput', false));

k_range=2:10;
n_folds=50;
[Consensus] = CAP_ConsensusClustering({representatives}, k_range, 'items', 0.9, n_folds, 'correlation')
% Perform kmeans and consensus clustering, just like normally with CAPs!

% MISSING NOW: - A proper build file. This build file should:
%  - install libzmq if it is not already installed
%  - install Eigen
%  - install