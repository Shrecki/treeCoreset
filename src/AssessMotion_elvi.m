%% This function evaluates the extent of motion of the considered subject
% in the scanner. Different methods exist to quantify motion, and the
% desired approach can be selected amongst a set of options.
%
% Inputs:
% - Path is the full path towards the considered subject (leading to the
% folder containing the motion file)
% - param is a structure with all relevant parameters; in this function, is
% must contain: 'Folder_motion' (name of the folder containing the
% motion file; if directly in Path, use []), 'TA_mot_prefix' (prefix of
% the motion file that should be read), 'skipped_scans' (number of scans
% that are not considered due to T1 equilibration effects), 'FD_method'
% (type of framewise displacement method to use; for now, use 'Power'),
% 'FD_threshold' (in mm, threshold past which instantaneous motion is
% considered too large)
% - fid points towards a log file in which we want to write the performed
% steps
%
% Outputs:
% - TemporalMask is a 1 x n_frames logical vector filled with '1' if a
% frame is kept, and '0' if to be scrubbed out because of excessive motion
function [TemporalMask] = AssessMotion_elvi(Path,soi,param,fid)


    if ~isfield(param,'Folder_motion')
        param.Folder_motion=[];
    end
    
    % compatibility with input as cell of strings for every subject or for a
    % simple string with the same subfoldeer for all subjects
    if ~isempty(param.Folder_motion) && iscell(param.Folder_motion)
        if length(param.Folder_motion)~=param.n_subjects
            error('param.Folder_motion: wrong number of motion subfolders');
        end
        param.Folder_motion=param.Folder_motion{soi};
    end
    
    if ~isempty(param.TA_mot_prefix) && iscell(param.TA_mot_prefix)
        if length(param.TA_mot_prefix)~=param.n_subjects
            error('param.TA_mot_prefix: wrong number of motion prefix names');
        end
        param.TA_mot_prefix=param.TA_mot_prefix{soi};
    end
    
    % Checks that the input folder and the folders containing the data
    % exist
    if ~exist(fullfile(Path,param.Folder_motion),'dir')
        error('There is no motion folder...');
    end
    
    if ~isfield(param,'skipped_scans_motionfile') || isempty(param.skipped_scans_motionfile)
        param.skipped_scans_motionfile=param.skipped_scans;
    end
    
    % Reads the motion file
    tmp_data = cellstr(spm_select('List',fullfile(Path,param.Folder_motion),['^' param.TA_mot_prefix '.*\.' 'txt' '$']));
    
    if length(tmp_data) ~= 1
        error('More than 1 motion file (or no motion file)...');
    else
        RP = textread(fullfile(Path,param.Folder_motion,tmp_data{1}));
    end

    % Removes the first scans
    RP=RP(param.skipped_scans_motionfile+1:end,:);

    % Converts the rotational displacement values into [mm] (are given
    % as [rad] by SPM)
    RP(:,4:6)=RP(:,4:6)*180/pi;

    switch param.FD_method
        case 'Power'
            
            %Calculate FD Power (Power, J.D., Barnes, K.A., Snyder, A.Z., 
            % Schlaggar, B.L., Petersen, S.E., 2012. Spurious but systematic 
            % correlations in functional connectivity MRI networks arise from 
            % subject motion. Neuroimage 59, 2142-2154.) 
            
            % Frame-to-frame change in motion
            RPDiff=diff(RP);
            RPDiff=[zeros(1,6);RPDiff];
            
            % Converts into [mm] displacement around a 50mm radius sphere
            RPDiffSphere=RPDiff;
            RPDiffSphere(:,4:6)=RPDiffSphere(:,4:6)*50*pi/180; % displacement on surface of sphere with radius 50 mm, arc length = theta(rad) * r
            
            % Computes FD_Power itself as the sum of absolute valued
            % elements
            FD_Power=sum(abs(RPDiffSphere),2);
            
            % Average FD_Power
            MeanFD_Power = mean(FD_Power);

            PercentFD_Power = length(find(FD_Power>param.FD_threshold)) / length(FD_Power) * 100;
           
            TemporalMask=ones(size(RP,1),1);
            TemporalMask(FD_Power > param.FD_threshold) = 0;
            TemporalMask = logical(TemporalMask);
           
            if ~isempty(fid)
                fprintf(fid,['Average motion:  ',num2str(MeanFD_Power),' [mm]']);
                fprintf(fid,['Motion: there are ',num2str(PercentFD_Power),'percent of data scrubbed']);
            end
        otherwise
            error('Scrubbing method not recognized - only FD Power is implemented so far for scubbing...');      

    end
end
