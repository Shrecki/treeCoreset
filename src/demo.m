
V=niftiread('/home/guibertf/Documents/Freya/2021-Esterman_InOutZone/niftifiles_GVA_2020/derivatives/preprocessed/sub-13/ses-01/func/sub-13_ses-01_task-gradCPT_bold_mni.nii');
brain = logical(spm_read_vols(spm_vol('/home/guibertf/Documents/Freya/2021-Esterman_InOutZone/niftifiles_GVA_2020/derivatives/preprocessed/sub-13/ses-01/func/MNI152_T1_2mm_brain_mask.nii')));
n_points=size(V,4);
%%
V_vec = zeros(sum(sum(sum(brain))),n_points, 'double');

for i=1:n_points
    vol=V(:,:,:,i);
    V_vec(:,i)=vol(brain(:));
end
%%
mu = mean(V_vec, 2);
sig = std(V_vec, 0, 2);
V_vec = V_vec - mu;
V_vec = V_vec*1./sig;
%% It is critical NaNs be removed. Indeed the C++ code WILL throw errors upon any NaN in the data, so sanitization is paramount!
nan_mask = ~isnan(V_vec(:,1));
%%
% Now send all of these to the representation

for i=1:n_points
    sendPoint(V_vec(nan_mask, i));
end

%% Now we can get the representation. To do such a thing, we can get the representatives and perform kMeans on them! We can specify several different cluster sizes, which saves computations.
mycell = cell(1,2);
[mycell{:}]=getKclusters(2); % This showcases how to get here two clusters
